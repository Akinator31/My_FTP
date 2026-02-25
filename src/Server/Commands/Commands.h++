//
// Created by pavel on 24/02/2026.
//

#pragma once
#include <string>

#include "Server/Server.h++"

namespace MyFtp {
    class Commands {
    public:
        static void user(Client& client, const std::string& command);
        static void pass(Client& client, const std::string& command);
        static void cwd(Client& client, const std::string& command);
    };
}
