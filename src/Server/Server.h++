//
// Created by pavel on 19/02/2026.
//

#pragma once
#include <string>
#include <vector>
#include <poll.h>
#include <netinet/in.h>

namespace my_ftp {
    class Server {
        size_t _port;
        std::string _path;
        int _serverSocket;
        sockaddr_in _serverSocketConfiguration{};
        std::vector<pollfd> _controlSocketsList;

        void _bind();
        void _listen() const;

    public:
        Server(size_t port, const std::string& path);

        void start();
    };
}
