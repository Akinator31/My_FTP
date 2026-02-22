//
// Created by pavel on 19/02/2026.
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

#include "Server.h++"

#include <iostream>
#include <unistd.h>

#include "Errors/MyFtpErrors.h++"

namespace my_ftp {
    Server::Server(const size_t port, const std::string& path) {
        this->_port = port;
        this->_path = path;
        this->_serverSocket = socket(AF_INET, SOCK_STREAM, 0);

        if (this->_serverSocket == -1)
            throw MyFtpErrors(ErrorCreateSocket);

        this->_controlSocketsList.push_back(
            {
                .fd = this->_serverSocket,
                .events = POLLIN,
                .revents = 0,
            }
        );

        this->_serverSocketConfiguration = {
            .sin_family = AF_INET,
            .sin_port = htons(this->_port),
            .sin_addr = {
                .s_addr = INADDR_ANY
            },
            .sin_zero = {}
        };

        this->_bind();
        this->_listen();
    }

    void Server::_bind() {
        if (bind(this->_serverSocket, reinterpret_cast<sockaddr*>(&this->_serverSocketConfiguration),
                 sizeof(this->_serverSocketConfiguration)) == -1)
            throw MyFtpErrors(ErrorBindSocket);
    }

    void Server::_listen() const {
        if (listen(this->_serverSocket, SOMAXCONN) == -1) {
            throw MyFtpErrors(ErrorListenSocket);
        }
    }

    void Server::start() {
        while (true) {
            if (poll(this->_controlSocketsList.data(), this->_controlSocketsList.size(), -1) == -1)
                throw MyFtpErrors(ErrorPollSocket);

            for (size_t i = 0; i < this->_controlSocketsList.size(); i++) {
                std::cout << "Socket : " << this->_controlSocketsList[i].fd << std::endl;
                if (this->_controlSocketsList[i].fd == this->_serverSocket && this->_controlSocketsList[i].revents &
                    POLLIN) {
                    std::cout << "New connection!" << std::endl;
                    socklen_t serverSocketConfigurationSize = sizeof(this->_serverSocketConfiguration);

                    if (const int newClientSocket = accept(
                        this->_controlSocketsList[i].fd, reinterpret_cast<sockaddr*>(&this->_serverSocketConfiguration),
                        &serverSocketConfigurationSize); newClientSocket == -1) {
                        throw MyFtpErrors(ErrorAcceptSocket);
                    }
                    else {
                        this->_controlSocketsList.push_back(
                            {
                                .fd = newClientSocket,
                                .events = POLLIN,
                                .revents = 0,
                            }
                        );
                        write(newClientSocket, "220 Service ready for new user.\r\n", 33);
                    }
                }
                else {
                    if (!(this->_controlSocketsList[i].revents & POLLIN))
                        continue;
                    char buffer[1024];

                    ssize_t bytesRead = read(this->_controlSocketsList[i].fd, buffer, sizeof(buffer) - 1);
                    if (bytesRead > 0) {
                        buffer[bytesRead] = '\0';
                        std::cout << "Data received on client " << this->_controlSocketsList[i].fd << " : " << buffer <<
                            std::endl;

                        write(this->_controlSocketsList[i].fd, "500 Unknown command\r\n", 21);
                    }
                    else if (bytesRead == 0) {
                        std::cout << "Client disconnected" << std::endl;
                        close(this->_controlSocketsList[i].fd);
                        this->_controlSocketsList.erase(this->_controlSocketsList.begin() + i);
                        i--;
                    }
                }
            }
        }
    }
}
