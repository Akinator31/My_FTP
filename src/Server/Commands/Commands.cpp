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

        if (!(ss >> commandName >> username)) {
            client.sendReply(500);
        }
        else {
            if (client.getUsername().empty()) {
                client.getUsername() = username;
            }
        }
    }
}
