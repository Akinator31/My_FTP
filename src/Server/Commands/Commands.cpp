//
// Created by pavel on 24/02/2026.
//

#include "Commands.h++"

#include <sstream>
#include <string>

#include "Server/Server.h++"

namespace MyFtp {
    void Commands::user(Client& client, const std::string& command) {
        std::istringstream ss(command);

        std::string commandName;
        std::string username;

        if (client.isClientAlreadyLoggedIn()) {
            client.sendReply(USER_LOGGED_IN_230);
            return;
        }
        if (!(ss >> commandName >> username)) {
            client.sendReply(SYNTAX_ERROR_ARGS_501);
        }
        else {
            client.getUsername() = username;
            client.sendReply(USERNAME_OK_331);
        }
    }

    void Commands::pass(Client& client, const std::string& command) {
        std::istringstream ss(command);

        std::string commandName;
        std::string password;

        if (client.isClientAlreadyLoggedIn()) {
            client.sendReply(USER_LOGGED_IN_230);
            return;
        }
        ss >> commandName >> password;

        client.getPassword() = password;
        if (client.getUsername().empty()) {
            client.sendReply(NEED_ACCOUNT_332);
            return;
        }
        if (client.getUsername() == "Anonymous" && password.empty()) {
            client.sendReply(USER_LOGGED_IN_230);
            client.userLoggedIn();
        }
        else
            client.sendReply(NOT_LOGGED_IN_530);
    }
}
