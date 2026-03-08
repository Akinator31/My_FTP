/**
 * @file Utils.h++
 * @brief Utility functions for argument parsing, path validation and command execution.
 * @date 19/02/2026
 * @author pavel
 */

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
         * @brief Parses the PORT command arguments into 6 integer values.
         *
         * The PORT command format is "PORT h1,h2,h3,h4,p1,p2" where h1-h4
         * form the IP address and p1,p2 form the port (port = p1*256 + p2).
         *
         * @param command The raw PORT command string from the client.
         * @return An array of 6 integers {h1, h2, h3, h4, p1, p2}, or std::nullopt if parsing fails.
         */
        static std::optional<std::array<int, 6>> parsePORTCommand(const std::string& command);

        /**
         * @brief Executes a shell command and captures its standard output.
         *
         * The output lines are converted to use CRLF line endings
         * as required by the FTP protocol.
         *
         * @param commandName The shell command to execute (e.g. "/bin/ls -l /tmp").
         * @return The command output as a string with CRLF line endings.
         */
        static std::string getOutputCommand(const std::string& commandName);
    };
}
