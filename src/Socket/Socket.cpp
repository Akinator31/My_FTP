//
// Created by pavel on 26/02/2026.
//

#include "Socket.h++"

#include <iostream>
#include <ostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "Errors/MyFtpErrors.h++"

namespace MyFtp {
    Socket::Socket() {
        this->_fd = socket(AF_INET, SOCK_STREAM, 0);

        if (this->_fd == -1)
            throw MyFtpErrors(ErrorCreateSocket);
    }

    Socket::Socket(const int socket) {
        this->_fd = socket;
    }

    Socket::~Socket() {
        close();
    }

    Socket::Socket(Socket&& other) noexcept : _fd(other._fd), _socketConfig(other._socketConfig) {
        other._fd = -1;
    }

    Socket& Socket::operator=(Socket&& other) noexcept {
        if (this != &other) {
            this->close();
            this->_fd = other._fd;
            this->_socketConfig = other._socketConfig;
            other._fd = -1;
        }
        return *this;
    }

    void Socket::close() {
        if (this->fd() != -1) {
            ::close(this->_fd);
            this->_fd = -1;
        }
    }

    void Socket::bind(const uint16_t port, const std::optional<sockaddr_in>& socketConfigOpt) {
        sockaddr_in socketConfig = {};

        if (socketConfigOpt.has_value()) {
            socketConfig = socketConfigOpt.value();
        } else {
            socketConfig = {
                .sin_family = AF_INET,
                .sin_port = htons(port),
                .sin_addr = {
                    .s_addr = INADDR_ANY
                },
                .sin_zero = {}
            };
        }

        const auto* castSocketConfig = reinterpret_cast<sockaddr*>(&socketConfig);
        constexpr socklen_t castSocketConfigSize = sizeof(socketConfig);

        if (::bind(this->_fd, castSocketConfig, castSocketConfigSize) == -1)
            throw MyFtpErrors(ErrorBindSocket);

        this->_socketConfig = socketConfig;
    }

    void Socket::listen() const {
        if (::listen(this->_fd, SOMAXCONN) == -1) {
            throw MyFtpErrors(ErrorListenSocket);
        }
    }

    int Socket::connect(const std::string& ip, const unsigned short port) const {
        sockaddr_in socketConfig{};

        socketConfig.sin_family = AF_INET;
        socketConfig.sin_port = htons(port);

        if (inet_pton(AF_INET, ip.data(), &socketConfig.sin_addr) <= 0) {
            std::cout << "ERROR" << std::endl;
            return -1;
        }
        return ::connect(this->_fd, reinterpret_cast<sockaddr*>(&socketConfig), sizeof(socketConfig));
    }

    Socket Socket::accept() const {
        sockaddr_in newSocketConfig{};
        socklen_t newSocketConfigSize = sizeof(newSocketConfig);
        const int newSocket = ::accept(this->_fd, reinterpret_cast<sockaddr*>(&newSocketConfig), &newSocketConfigSize);

        if (newSocket == -1)
            throw MyFtpErrors(ErrorAcceptSocket);

        Socket newS(newSocket);
        newS._socketConfig = newSocketConfig;

        return newS;
    }

    ssize_t Socket::read(void* buffer, const size_t size) const {
        return ::read(this->fd(), buffer, size);
    }

    ssize_t Socket::write(const char* buffer, const size_t size) const {
        return ::write(this->fd(), buffer, size);
    }

    int Socket::fd() const {
        return this->_fd;
    }

    sockaddr_in& Socket::getSin() {
        return this->_socketConfig;
    }
}
