//
// Created by pavel on 19/02/2026.
//

#pragma once
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "Client/Client.h++"
#include "FtpSession/FtpSession.h++"
#include "Poller/Poller.h++"

namespace MyFtp {
    class SignalHandler {
    public:
        static bool mustClose;

        static void sigintHandler(int code);
    };

    class Server {
        std::string _path;
        FtpSession _serverSession;

        std::vector<Client> _clients;
        std::map<std::string, std::function<void (Client&, const std::string&)>> _funcMap;

        Poller _poller;

        void _acceptClientConnection();
        void _disconnectClient(size_t& clientIndex, bool needToClose);
        void _handleCommand(Client& client, const std::string& command);

    public:
        Server(size_t port, const std::string& path);

        void start();
    };
}
