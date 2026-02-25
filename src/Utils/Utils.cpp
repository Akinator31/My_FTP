//
// Created by pavel on 19/02/2026.
//

#include <sstream>
#include <filesystem>
#include "Utils.h++"

#include <iostream>

#include "Errors/MyFtpErrors.h++"

namespace MyFtp {
    int Utils::printUsage() {
        std::cout << "USAGE: ./myftp port path" << std::endl;
        std::cout << " port is the port number on which the server socket listens" << std::endl;
        std::cout << " path is the path to the home directory for the Anonymous user" << std::endl;
        return 0;
    }

    Server Utils::loadServer(char** av) {
        size_t port = 0;
        const std::string path(av[2]);
        std::stringstream rawPort(av[1]);

        rawPort >> port;

        if (rawPort.fail())
            throw MyFtpErrors(IncorrectPort);
        if (!std::filesystem::exists(av[2]))
            throw MyFtpErrors(IncorrectPath);

        Server server(port, path);
        return server;
    }

    bool Utils::isPathInsideTheRootPath(const std::filesystem::path& rootPath, const std::filesystem::path& path) {
        try {
            const auto relativePath = std::filesystem::relative(path, rootPath);

            return !relativePath.string().starts_with("..");
        }
        catch (const std::filesystem::filesystem_error&) {
            return false;
        }
    }
}
