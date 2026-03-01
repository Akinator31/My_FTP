//
// Created by pavel on 24/02/2026.
//

#pragma once
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <sys/poll.h>

#include "FtpSession/FtpSession.h++"

namespace MyFtp {
    /**
     * @enum replyCode
     * @brief All the FTP reply codes that the server can send back to the client.
     *
     * Each value correspond to a standard FTP reply code from the RFC 959.
     */
    enum replyCode {
        COMMAND_OK_200, ///< The command was executed succesfully.
        SERVICE_READY_220, ///< The server is ready for a new user.
        SERVICE_CLOSING_221, ///< The server is closing the connection.
        USER_LOGGED_IN_230, ///< The user is now logged in.
        REQUEST_FILE_ACTION_OK_250, ///< The requested file action was completed.

        USERNAME_OK_331, ///< Username is ok, now we need the password.
        NEED_ACCOUNT_332, ///< We need a account to login.

        SYNTAX_ERROR_COMMAND_500, ///< The command was not reconized.
        SYNTAX_ERROR_ARGS_501, ///< The arguments of the command are wrong.
        NOT_LOGGED_IN_530, ///< The user is not logged in yet.
        FILE_UNAVAILABLE_550, ///< The requested file is not available or doesnt exist.
    };

    /**
     * @class Client
     * @brief Represents a single FTP client that is connected to the server.
     *
     * This class store everything about a connected client: its socket session,
     * the login informations, the current working directory, and the reply messages.
     * It also handles reading commands from the client and sending replys back.
     */
    class Client {
        int _fd;
        std::unique_ptr<FtpSession> _session;
        std::string _username = {};
        std::string _password = "placeHolder";
        std::filesystem::path _rootPath;
        std::filesystem::path _currentPath;
        bool _mustLogOff = false;
        bool _isClientLoggedIn = false;

        std::map<replyCode, std::string> _replyMessage = {
            {COMMAND_OK_200, "200 Command okay.\r\n"},
            {SERVICE_READY_220, "220 Service ready for new user.\r\n"},
            {SERVICE_CLOSING_221, "221 Service closing control connection.\r\n"},
            {USER_LOGGED_IN_230, "230 User logged in, proceed.\r\n"},
            {REQUEST_FILE_ACTION_OK_250, "250 Requested file action okay, completed.\r\n"},

            {USERNAME_OK_331, "331 User name okay, need password.\r\n"},
            {NEED_ACCOUNT_332, "332 Need account for login.\r\n"},

            {SYNTAX_ERROR_COMMAND_500, "500 Syntax error, command unrecognized.\r\n"},
            {SYNTAX_ERROR_ARGS_501, "501 Syntax error in parameters or arguments.\r\n"},
            {NOT_LOGGED_IN_530, "530 Not logged in.\r\n"},
            {FILE_UNAVAILABLE_550, "550 Requested action not taken.\r\n"},
        };

    public:
        /**
         * @enum ReadResult
         * @brief The possible results when you try to read data from a client.
         */
        enum ReadResult {
            Ok, ///< Data was readed succesfully.
            Disconnected, ///< The client has disconnected.
            Error ///< Something went wrong while reading.
        };

        /**
         * @brief Creates a new Client with its socket, session and root path.
         * @param fd The file descriptor of the client socket.
         * @param session A unique pointer to the FtpSession of this client.
         * @param rootPath The root directory path that the client is allowed to acces.
         */
        Client(int fd, std::unique_ptr<FtpSession> session, const std::string& rootPath);

        /**
         * @brief Sends a FTP reply to the client based on the reply code.
         * @param code The reply code to send (like COMMAND_OK_200 or NOT_LOGGED_IN_530).
         * @throws MyFtpErrors if the reply code dosnt exist in the map.
         */
        void sendReply(replyCode code);

        /**
         * @brief Gets the FTP session of this client.
         * @return A reference to the unique pointer of the session.
         */
        std::unique_ptr<FtpSession>& getSession();

        /**
         * @brief Gets the username of this client.
         * @return A reference to the username string.
         */
        std::string& getUsername();

        /**
         * @brief Gets the password of this client.
         * @return A reference to the password string.
         */
        std::string& getPassword();

        /**
         * @brief Gets the root path that the client cant go above.
         * @return A reference to the root path.
         */
        std::filesystem::path& getRootPath();

        /**
         * @brief Gets the current working directory of the client.
         * @return A reference to the current path.
         */
        std::filesystem::path& getCurrentPath();

        /**
         * @brief Marks the client for disconnection. It will be disconnected on the next loop.
         */
        void disconnect();

        /**
         * @brief Changes the current working directory of the client.
         * @param path The new path to set as current directory.
         */
        void setCurrentPath(const std::filesystem::path& path);

        /**
         * @brief Marks the client as logged in. After this, the client can use all the commands.
         */
        void userLoggedIn();

        /**
         * @brief Checks if the client needs to be disconnected.
         * @return true if the client should be disconnected, false if not.
         */
        [[nodiscard]] bool mustLogOff() const;

        /**
         * @brief Checks if the client is already logged in.
         * @return true if the client is logged in, false if not.
         */
        [[nodiscard]] bool isClientAlreadyLoggedIn() const;

        /**
         * @brief Reads incomming data from the client socket into the command buffer.
         * @return Ok if data was readed, Disconnected if the client left, Error if something went wrong.
         */
        ReadResult readIncoming();

        /**
         * @brief Gets the next complete command from the command buffer.
         *
         * A command is considered complete when it ends with "\\r\\n".
         * The command is removed from the buffer after its returned.
         *
         * @return The next command as a string, or std::nullopt if no complete command is available.
         */
        [[nodiscard]] std::optional<std::string> nextCommand() const;

        /**
         * @brief Sends all the data that is waiting in the output buffer to the client.
         *
         * If the output buffer is empty, this function does nothing.
         */
        void flushOutput();
    };
}
