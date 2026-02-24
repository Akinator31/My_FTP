//
// Created by pavel on 24/02/2026.
//

#pragma once
#include <memory>
#include <string>
#include <sys/poll.h>

#include "FtpSession/FtpSession.h++"

namespace MyFtp {
    class Client {
        pollfd _pfd;
        std::unique_ptr<FtpSession> _session;
        std::string _username = {};
        std::string _password = {};

        std::map<int, std::string> _replyMessage = {
            {220, "220 Service ready for new user.\r\n"},
            {500, "500 Syntax error, command unrecognized\r\n"},
        };

    public:
        Client(pollfd pfd, std::unique_ptr<FtpSession> session);
        void sendReply(int replyCode);

        pollfd& getPfd();
        std::unique_ptr<FtpSession>& getSession();
        std::string& getUsername();
        std::string& getPassword();
    };
}
