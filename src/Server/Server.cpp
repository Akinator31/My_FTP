//
// Created by pavel on 19/02/2026.
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <poll.h>
#include <iostream>
#include <memory>
#include <unistd.h>

#include "Server.h++"

#include <csignal>
#include <sstream>
#include <criterion/logging.h>

#include "Client/Client.h++"
#include "Commands.h++"
#include "FtpSession/FtpSession.h++"
#include "Errors/MyFtpErrors.h++"

namespace MyFtp {
    bool SignalHandler::mustClose{false};

    void SignalHandler::sigintHandler([[maybe_unused]] const int code) {
        std::cout << "Shutting down my_ftp server..." << std::endl;
        mustClose = true;
    }

    Server::Server(const size_t port, const std::string& path) : _serverSession(
        FtpSession(FTPServer, Socket())) {
        this->_path = path;

        this->_funcMap = {
            {"USER", &Commands::user},
            {"PASS", &Commands::pass},
            {"CWD", &Commands::cwd},
            {"CDUP", &Commands::cdup},
            {"QUIT", &Commands::quit},
            {"PWD", &Commands::pwd},
            {"NOOP", &Commands::noop},
            {"HELP", &Commands::help},
            {"DELE", &Commands::dele},
            {"PASV", &Commands::pasv},
            {"PORT", &Commands::port},
            {"LIST", &Commands::list},
        };

        this->_poller.add(this->_serverSession.getControlSocket().fd(), POLLIN);

        this->_serverSession.getControlSocket().bind(port);
        this->_serverSession.getControlSocket().listen();
    }

    void Server::_acceptClientConnection() {
        sockaddr_in clientConfig{};
        socklen_t clientAddrLen = sizeof(clientConfig);

        const int newClientSocket = accept(
            this->_serverSession.getControlSocket().fd(), reinterpret_cast<sockaddr*>(&clientConfig),
            &clientAddrLen);

        if (newClientSocket == -1) {
            throw MyFtpErrors(ErrorAcceptSocket);
        }

        this->_poller.add(newClientSocket, POLLIN | POLLOUT);

        Socket newSocket(newClientSocket);
        newSocket.getSin() = clientConfig;

        this->_clients.emplace_back(newClientSocket,
                                    std::make_unique<FtpSession>(FTPClient, std::move(newSocket)), this->_path);

        this->_clients.back().sendReply(SERVICE_READY_220);
    }

    void Server::_disconnectClient(size_t& clientIndex, const bool needToClose) {
        if (needToClose)
            close(this->_clients[clientIndex].getSession()->getControlSocket().fd());
        this->_clients.erase(this->_clients.begin() + static_cast<int>(clientIndex));
        clientIndex--;
    }

    void Server::_handleCommand(Client& client, const std::string& command) {
        std::stringstream commandSs(command);
        std::string name = {};
        std::string rest = {};

        commandSs >> name;

        std::cout << "COMMAND : " << command << std::endl;

        if (this->_funcMap.contains(name)) {
            this->_funcMap[name](client, command);
        } else {
            client.sendReply(SYNTAX_ERROR_COMMAND_500);
        }
    }

    void Server::start() {
        signal(SIGINT, SignalHandler::sigintHandler);

        while (!SignalHandler::mustClose) {
            if (this->_poller.wait() == -1) {
                if (SignalHandler::mustClose)
                    break;
                throw MyFtpErrors(ErrorPollSocket);
            }

            if (this->_poller.isReadable(this->_serverSession.getControlSocket().fd())) {
                this->_acceptClientConnection();
                continue;
            }

            for (size_t i = 0; i < this->_clients.size(); i++) {
                Client& client = this->_clients[i];
                DataTransferManager& manager = client.getDataTransferManager();
                const int clientFd = client.getSession()->getControlSocket().fd();

                this->_poller.handleAwaitingDataConnection(client);

                if (manager.updateDataTransfer() == CLOSING)
                    client.sendReply(CLOSING_DATA_226);

                if (this->_poller.isReadable(clientFd)) {
                    const auto result = this->_clients[i].readIncoming();
                    if (result == Client::ReadResult::Disconnected) {
                        this->_poller.remove(clientFd);
                        this->_disconnectClient(i, true);
                        continue;
                    }
                    if (result == Client::ReadResult::Error)
                        throw MyFtpErrors(ErrorReadSocket);

                    while (auto cmd = this->_clients[i].nextCommand()) {
                        this->_handleCommand(this->_clients[i], *cmd);
                    }
                }

                if (this->_poller.isWritable(clientFd))
                    this->_clients[i].flushOutput();

                if (this->_poller.isInvalid(clientFd) ||
                    this->_poller.hasError(clientFd) ||
                    this->_poller.hasHangup(clientFd) ||
                    this->_clients[i].mustLogOff()) {
                    this->_poller.remove(clientFd);
                    this->_disconnectClient(i, false);
                }
            }
        }
    }
}
