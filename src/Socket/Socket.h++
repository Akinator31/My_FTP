//
// Created by pavel on 26/02/2026.
//

#pragma once
#include <netinet/in.h>

namespace MyFtp {
    class Socket {
        int _fd;
        sockaddr_in _socketConfig{};

    public:
        Socket();
        explicit Socket(int socket);
        ~Socket();
        Socket(Socket&& other) noexcept;
        Socket& operator=(Socket&& other) noexcept;
        Socket(const Socket&) = delete;
        Socket& operator=(const Socket&) = delete;

        void bind(uint16_t port);
        void listen() const;
        [[nodiscard]] Socket accept() const;
        [[nodiscard]] ssize_t read(void* buffer, size_t size) const;
        ssize_t write(const char* buffer, size_t size) const;
        void close();

        [[nodiscard]] int fd() const;
    };
}
