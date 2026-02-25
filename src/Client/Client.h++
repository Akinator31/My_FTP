//
// Created by pavel on 24/02/2026.
//

#pragma once
#include <filesystem>
#include <memory>
#include <string>
#include <sys/poll.h>

#include "FtpSession/FtpSession.h++"

namespace MyFtp {
    enum replyCode {
        COMMAND_OK_200,
        SERVICE_READY_220,
        USER_LOGGED_IN_230,
        REQUEST_FILE_ACTION_OK_250,

        USERNAME_OK_331,
        NEED_ACCOUNT_332,

        SYNTAX_ERROR_COMMAND_500,
        SYNTAX_ERROR_ARGS_501,
        NOT_LOGGED_IN_530,
        FILE_UNAVAILABLE_550,
    };

    class Client {
        pollfd _pfd;
        std::unique_ptr<FtpSession> _session;
        std::string _username = {};
        std::string _password = "placeHolder";
        std::filesystem::path _rootPath;
        std::filesystem::path _currentPath;
        bool _isClientLoggedIn = false;

        std::map<replyCode, std::string> _replyMessage = {
            {COMMAND_OK_200, "200 Command okay.\r\n"},
            {SERVICE_READY_220, "220 Service ready for new user.\r\n"},
            {USER_LOGGED_IN_230, "230 User logged in, proceed.\r\n"},
            {REQUEST_FILE_ACTION_OK_250, "250 Requested file action okay, completed.\r\n"},

            {USERNAME_OK_331, "331 User name okay, need password.\r\n"},
            {NEED_ACCOUNT_332, "332 Need account for login.\r\n"},

            {SYNTAX_ERROR_COMMAND_500, "500 Syntax error, command unrecognized.\r\n"},
            {SYNTAX_ERROR_ARGS_501, "501 Syntax error in parameters or arguments.\r\n"},
            {NOT_LOGGED_IN_530, "530 Not logged in.\r\n"},
            {FILE_UNAVAILABLE_550, "550 Requested action not taken.\r\n"},

        };

    public:
        Client(pollfd pfd, std::unique_ptr<FtpSession> session, const std::string& rootPath);
        void sendReply(replyCode code);

        pollfd& getPfd();
        std::unique_ptr<FtpSession>& getSession();
        std::string& getUsername();
        std::string& getPassword();
        std::filesystem::path& getRootPath();
        std::filesystem::path& getCurrentPath();
        void setCurrentPath(const std::filesystem::path& path);
        void userLoggedIn();
        [[nodiscard]] bool isClientAlreadyLoggedIn() const;
    };
}
