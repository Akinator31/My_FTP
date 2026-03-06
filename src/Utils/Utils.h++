//
// Created by pavel on 19/02/2026.
//

#pragma once
#include "Server/Server.h++"

namespace MyFtp {
    /**
     * @class Utils
     * @brief Some utility functions used by the server at startup.
     *
     * This class contains static helper functions for parsing the command line
     * arguments, creating the server object and checking paths.
     */
    class Utils {
    public:
        /**
         * @brief Parses the command line arguments and creates a Server object.
         *
         * It reads the port and path from the arguments, validates them
         * and then returns a ready-to-use Server.
         *
         * @param av The command line arguments array (av[1] = port, av[2] = path).
         * @return A Server object configured with the given port and path.
         * @throws MyFtpErrors if the port is invalid or the path dosnt exist.
         */
        static Server loadServer(char** av);

        /**
         * @brief Prints the usage/help message to the terminal and returns 0.
         * @return Always returns 0.
         */
        static int printUsage();

        /**
         * @brief Checks if a path is inside the root path (so the client dosnt escape).
         *
         * This is used to make sure that a client cant navigate outside
         * of the FTP root directory using things like "../../".
         *
         * @param rootPath The root path that the client should not go above.
         * @param path The path that we want to check.
         * @return true if the path is inside the root, false if it goes outside.
         */
        static bool isPathInsideTheRootPath(const std::filesystem::path& rootPath, const std::filesystem::path& path);

        /**
         * Parse the PORT command from a client.
         * @return A pair with IP as a string and the port as a short.
         */
        static std::optional<std::array<int, 6>> parsePORTCommand(const std::string& command);

        static std::string getOutputCommand(const std::string& commandName);
    };
}
