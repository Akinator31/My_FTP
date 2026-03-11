//
// Created by pavel on 19/02/2026.
//

#include <sstream>
#include <filesystem>
#include "Utils.h++"

#include <format>
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

    std::optional<std::array<int, 6>> Utils::parsePORTCommand(const std::string& command) {
        std::array<int, 6> result{};

        std::string_view line(command);

        if (line.substr(0, 5) == "PORT ")
            line.remove_prefix(5);

        for (int i = 0; i < 6; i++) {
            auto [ptr, errorCode] = std::from_chars(line.data(), line.data() + line.size(), result[i]);

            if (errorCode != std::errc()) {
                return std::nullopt;
            }

            const size_t consumed = ptr - line.data();

            if (i < 5)
                line.remove_prefix(consumed + 1);
        }

        return result;
    }

    std::string Utils::getOutputCommand(const std::string& commandName) {
        std::string result;
        FILE* pipe = popen(commandName.c_str(), "r");

        if (!pipe)
            return "FAILED TO POPEN";

        char buffer[1024];

        while (fgets(buffer, sizeof(buffer), pipe)) {
            result += buffer;
            if (result.ends_with("\n"))
                result.pop_back();
            result += "\r\n";
        }
        pclose(pipe);
        return result;
    }
}