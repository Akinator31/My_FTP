//
// Created by pavel on 24/02/2026.
//

#include "Commands.h++"

#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

#include "Server/Server.h++"
#include "Utils/Utils.h++"

namespace MyFtp {
    void Commands::user(Client& client, const std::string& command) {
        std::istringstream ss(command);

        std::string commandName;

        if (client.isClientAlreadyLoggedIn()) {
            client.sendReply(USER_LOGGED_IN_230);
            return;
        }
        if (std::string username; !(ss >> commandName >> username)) {
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

    void Commands::cwd(Client& client, const std::string& command) {
        std::istringstream ss(command);

        std::string directory;

        if (std::string commandName; !(ss >> commandName >> directory)) {
            client.sendReply(SYNTAX_ERROR_ARGS_501);
        }

        const std::filesystem::path combinedPath = client.getCurrentPath() / directory;
        const std::filesystem::path normalizedPath = combinedPath.lexically_normal();

        try {
            if (Utils::isPathInsideTheRootPath(client.getRootPath(), canonical(normalizedPath))) {
                client.setCurrentPath(canonical(normalizedPath));

                client.sendReply(REQUEST_FILE_ACTION_OK_250);
            }
            else {
                client.sendReply(FILE_UNAVAILABLE_550);
            }
        }
        catch (std::filesystem::filesystem_error&) {
            client.sendReply(FILE_UNAVAILABLE_550);
        }
    }
}
