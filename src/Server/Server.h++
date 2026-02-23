//
// Created by pavel on 19/02/2026.
//

#pragma once
#include <functional>
#include <map>
#include <string>
#include <vector>
#include <poll.h>
#include <memory>

#include "FtpSession/FtpSession.h++"

namespace my_ftp {
    struct ClientNode {
        pollfd pfd;
        std::unique_ptr<FtpSession> session;
    };

    class Server {
        std::string _path;
        FtpSession _serverSession;

        std::vector<ClientNode> _clients;
        std::map<std::string, std::function<void ()>> _funcMap;

        void _bind();
        void _listen() const;

        void _acceptClientConnection();
        [[nodiscard]] bool _isServerSocketForPollIn(const pollfd& socket) const;
        void _disconnectClient(size_t& clientIndex);
        void _handleCommand(const ClientNode& client, const std::string& command);

    public:
        Server(size_t port, const std::string& path);

        void start();
    };
}
