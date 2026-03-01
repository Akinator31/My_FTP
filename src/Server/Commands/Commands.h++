//
// Created by pavel on 24/02/2026.
//

#pragma once
#include <string>

#include "Server/Server.h++"

namespace MyFtp {
    /**
     * @class Commands
     * @brief Contains all the FTP command handlers.
     *
     * Each static function handles one FTP command. They all take a Client
     * reference and the raw command string, then they parse the arguments
     * and do the thing that the command is supposed to do.
     */
    class Commands {
        /**
         * @brief Trys to change the working directory of a client.
         *
         * It checks if the new path is still inside the root directory
         * so the client cant escape to other parts of the filesystem.
         *
         * @param client The client whos directory we want to change.
         * @param path The new path to set.
         * @return 1 if the directory was changed succesfully, 0 if it failed.
         */
        static int _setWorkingDirectory(Client& client, const std::filesystem::path& path);

        static inline const std::map<std::string, std::string> _helpFuncMessages = {
            {"USER", "USER <username> - Specify the username for authentication."},
            {"PASS", "PASS <password> - Specify the password for authentication."},
            {"CWD", "CWD <pathname> - Change the current working directory."},
            {"CDUP", "CDUP - Change to the parent directory."},
            {"QUIT", "QUIT - Close the connection."},
            {"PWD", "PWD - Print the current working directory."},
            {"NOOP", "NOOP - No operation (keeps the connection alive)."},
            {"HELP", "HELP [<command>] - Display help information."},
        };

        static inline const std::string _helpGlobalMessage =
            "The following commands are recognized:\n"
            " USER PASS CWD CDUP QUIT PWD NOOP HELP";

    public:
        /**
         * @brief Handles the USER command.
         *
         * Sets the username for the client. If the client is already logged in
         * it just sends a "already logged in" reply.
         *
         * @param client The client that sended the command.
         * @param command The full command string (like "USER Anonymous").
         */
        static void user(Client& client, const std::string& command);

        /**
         * @brief Handles the PASS command.
         *
         * Checks if the username was already given, then trys to log in.
         * Only anonymous login is suported (username "Anonymous" with no password).
         *
         * @param client The client that sended the command.
         * @param command The full command string (like "PASS" or "PASS mypassword").
         */
        static void pass(Client& client, const std::string& command);

        /**
         * @brief Handles the CWD (Change Working Directory) command.
         *
         * Changes the current directory of the client to the given path.
         * The client must be logged in, and the path must stay inside the root directory.
         *
         * @param client The client that sended the command.
         * @param command The full command string (like "CWD /some/directory").
         */
        static void cwd(Client& client, const std::string& command);

        /**
         * @brief Handles the CDUP command.
         *
         * Goes up one directory (like doing "cd .."). The client must be logged in
         * and cant go above the root directory.
         *
         * @param client The client that sended the command.
         * @param command The full command string (should just be "CDUP" with no arguments).
         */
        static void cdup(Client& client, const std::string& command);

        /**
         * @brief Handles the QUIT command.
         *
         * Sends a "221 closing connection" reply and marks the client for disconnection.
         * The command should have no arguments.
         *
         * @param client The client that sended the command.
         * @param command The full command string (should just be "QUIT").
         */
        static void quit(Client& client, const std::string& command);

        /**
         * @brief Handles the PWD command.
         *
         * Sends a "257 "PATHNAME" created" reply.
         * The command should have no arguments.
         *
         * @param client The client that sended the command.
         * @param command The full command string (should just be "QUIT").
         */
        static void pwd(Client& client, const std::string& command);

        /**
        * @brief Handles the NOOP command.
        *
        * Do absolutely nothing.
        * The command should have no arguments.
        *
        * @param client The client that sended the command.
        * @param command The full command string (should just be "QUIT").
        */
        static void noop(Client& client, const std::string& command);

        /**
        * @brief Handles the HELP command.
        *
        * Sends a "214 Help message" reply
        * The command should have no arguments.
        *
        * @param client The client that sended the command.
        * @param command The full command string (should just be "QUIT").
        */
        static void help(Client& client, const std::string& command);
    };
}
