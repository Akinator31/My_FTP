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
    int Commands::_setWorkingDirectory(Client& client, const std::filesystem::path& path) {
        try {
            if (Utils::isPathInsideTheRootPath(client.getRootPath(), canonical(path))) {
                client.setCurrentPath(canonical(path));

                return 1;
            }
            return 0;
        }
        catch (std::filesystem::filesystem_error&) {
            return 0;
        }
    }

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

        if (!client.isClientAlreadyLoggedIn()) {
            client.sendReply(NOT_LOGGED_IN_530);
            return;
        }

        if (std::string commandName; !(ss >> commandName >> directory)) {
            client.sendReply(SYNTAX_ERROR_ARGS_501);
        }

        const std::filesystem::path combinedPath = client.getCurrentPath() / directory;
        std::filesystem::path normalizedPath = combinedPath.lexically_normal();

        if (_setWorkingDirectory(client, normalizedPath))
            client.sendReply(REQUEST_FILE_ACTION_OK_250);
        else
            client.sendReply(FILE_UNAVAILABLE_550);
    }

    void Commands::cdup(Client& client, const std::string& command) {
        std::istringstream ss(command);
        std::string rest;

        if (!client.isClientAlreadyLoggedIn()) {
            client.sendReply(NOT_LOGGED_IN_530);
            return;
        }

        if (std::string commandName; ss >> commandName >> rest) {
            client.sendReply(SYNTAX_ERROR_ARGS_501);
            return;
        }

        const std::filesystem::path combinedPath = client.getCurrentPath() / "../";
        std::filesystem::path normalizedPath = combinedPath.lexically_normal();

        if (_setWorkingDirectory(client, normalizedPath))
            client.sendReply(COMMAND_OK_200);
        else
            client.sendReply(FILE_UNAVAILABLE_550);
    }

    void Commands::quit(Client& client, const std::string& command) {
        std::istringstream ss(command);
        std::string rest;

        if (std::string commandName; ss >> commandName >> rest) {
            client.sendReply(SYNTAX_ERROR_ARGS_501);
            return;
        }

        client.sendReply(SERVICE_CLOSING_221);
        client.disconnect();
    }

    void Commands::pwd(Client& client, const std::string& command) {
        std::istringstream ss(command);
        std::stringstream output;
        std::string rest;

        if (!client.isClientAlreadyLoggedIn()) {
            client.sendReply(NOT_LOGGED_IN_530);
            return;
        }

        if (std::string commandName; ss >> commandName >> rest) {
            client.sendReply(SYNTAX_ERROR_ARGS_501);
            return;
        }

        output << "257 \"" << client.getVirtualPath() << "\" created.\n";
        client.sendReply(static_cast<replyCode>(0), output.str());
    }
}
