//
// Created by pavel on 19/02/2026.
//

#pragma once
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <poll.h>

#include "Client/Client.h++"
#include "FtpSession/FtpSession.h++"

namespace MyFtp {
    class Server {
        std::string _path;
        FtpSession _serverSession;

        std::vector<Client> _clients;
        std::map<std::string, std::function<void (Client&, const std::string&)>> _funcMap;

        void _bind();
        void _listen() const;

        void _acceptClientConnection();
        [[nodiscard]] bool _isServerSocketForPollIn(const pollfd& socket) const;
        void _disconnectClient(size_t& clientIndex);
        void _handleCommand(Client& client, const std::string& command);

    public:
        Server(size_t port, const std::string& path);

        void start();
    };
}
