//
// Created by pavel on 19/02/2026.
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>
#include <iostream>
#include <memory>
#include <unistd.h>

#include "Server.h++"

#include <sstream>

#include "Client/Client.h++"
#include "Commands/Commands.h++"
#include "FtpSession/FtpSession.h++"
#include "Errors/MyFtpErrors.h++"


namespace MyFtp {
    Server::Server(const size_t port, const std::string& path) : _serverSession(
        FtpSession(FTPServer, socket(AF_INET, SOCK_STREAM, 0))) {
        this->_path = path;

        this->_funcMap = {
            {"USER", &Commands::user},
            {"PASS", &Commands::pass},
            {"CWD", &Commands::cwd},
            {"CDUP", &Commands::cdup},
        };

        this->_serverSession.setSocketConfiguration(
            {
                .sin_family = AF_INET,
                .sin_port = htons(port),
                .sin_addr = {
                    .s_addr = INADDR_ANY
                },
                .sin_zero = {}
            }
        );

        this->_bind();
        this->_listen();
    }

    void Server::_bind() {
        const auto* serverConfiguration = reinterpret_cast<sockaddr*>(&this->_serverSession.getSocketConfiguration());
        socklen_t serverConfigurationSize = sizeof(this->_serverSession.getSocketConfiguration());

        if (bind(this->_serverSession.getControlSocket(), serverConfiguration, serverConfigurationSize) == -1)
            throw MyFtpErrors(ErrorBindSocket);
    }

    void Server::_listen() const {
        if (listen(this->_serverSession.getControlSocket(), SOMAXCONN) == -1) {
            throw MyFtpErrors(ErrorListenSocket);
        }
    }

    bool Server::_isServerSocketForPollIn(const pollfd& socket) const {
        if (socket.fd == this->_serverSession.getControlSocket() && socket.revents & POLLIN)
            return true;
        return false;
    }

    void Server::_acceptClientConnection() {
        sockaddr_in clientConfig{};
        socklen_t clientAddrLen = sizeof(clientConfig);

        const int newClientSocket = accept(
            this->_serverSession.getControlSocket(), reinterpret_cast<sockaddr*>(&clientConfig),
            &clientAddrLen);

        if (newClientSocket == -1) {
            throw MyFtpErrors(ErrorAcceptSocket);
        }

        this->_clients.push_back(Client({.fd = newClientSocket, .events = POLLIN | POLLOUT, .revents = 0},
                                        std::make_unique<FtpSession>(FTPClient, newClientSocket), this->_path));

        this->_clients.back().sendReply(SERVICE_READY_220);
    }

    void Server::_disconnectClient(size_t& clientIndex) {
        close(this->_clients[clientIndex].getPfd().fd);
        this->_clients.erase(this->_clients.begin() + static_cast<int>(clientIndex));
        clientIndex--;
    }

    void Server::_handleCommand(Client& client, const std::string& command) {
        std::stringstream commandSs(command);
        std::string name = {};
        std::string rest = {};

        commandSs >> name;

        if (this->_funcMap.contains(name)) {
            this->_funcMap[name](client, command);
        }
        else {
            client.sendReply(SYNTAX_ERROR_COMMAND_500);
            printf("%s\n", client.getSession()->getOutputBuffer().data());
        }
    }

    void Server::start() {
        while (true) {
            std::vector<pollfd> pfds;
            pfds.push_back({
                .fd = this->_serverSession.getControlSocket(),
                .events = POLLIN,
                .revents = 0,
            });

            for (auto& client : this->_clients)
                pfds.push_back(client.getPfd());

            if (poll(pfds.data(), pfds.size(), -1) == -1)
                throw MyFtpErrors(ErrorPollSocket);

            if (pfds[0].revents & POLLIN) {
                this->_acceptClientConnection();
                continue;
            }

            for (size_t i = 0; i < this->_clients.size(); i++) {
                if (pfds[i + 1].revents & POLLIN) {
                    char buffer[1024] = {};
                    std::string command = {};
                    ssize_t bytesRead = read(this->_clients[i].getPfd().fd, buffer, sizeof(buffer) - 1);

                    if (bytesRead > 0) {
                        buffer[bytesRead] = '\0';
                        this->_clients[i].getSession()->getCommandBuffer().append(buffer, bytesRead);

                        if (const size_t pos = this->_clients[i].getSession()->getCommandBuffer().find("\r\n"); pos !=
                            std::string::npos) {
                            command.append(this->_clients[i].getSession()->getCommandBuffer().substr(0, pos));
                            this->_clients[i].getSession()->getCommandBuffer().erase(0, pos + 2);
                            this->_handleCommand(this->_clients[i], command);
                            continue;
                        }
                    }
                    else if (bytesRead == 0) {
                        std::cout << "Client disconnected" << std::endl;
                        this->_disconnectClient(i);
                        continue;
                    }
                    else {
                        throw MyFtpErrors(ErrorReadSocket);
                    }
                }
                if (std::string& outputBuffer = this->_clients[i].getSession()->getOutputBuffer(); pfds[i + 1].revents &
                    POLLOUT && pfds[i + 1].fd != this->_serverSession.getControlSocket() && !outputBuffer.empty()) {
                    write(this->_clients[i].getPfd().fd, outputBuffer.data(), outputBuffer.size());
                    outputBuffer = "";
                }
            }
        }
    }
}
