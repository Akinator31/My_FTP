//
// Created by pavel on 19/02/2026.
//

#pragma once
#include <exception>
#include <string>
#include <map>

namespace MyFtp {
    /**
     * @enum MyFtpErrorType
     * @brief All the differents types of errors that can happen in the server.
     */
    enum MyFtpErrorType {
        IncorrectNumberArgs, ///< Wrong number of arguments was given.
        IncorrectArgs, ///< The arguments are invalid.
        IncorrectPort, ///< The port number is not valid.
        IncorrectPath, ///< The path dosnt exist or is not valid.
        ErrorCreateSocket, ///< Failed to create a socket.
        ErrorBindSocket, ///< Failed to bind the socket to a port.
        ErrorListenSocket, ///< Failed to put the socket in listen mode.
        ErrorPollSocket, ///< Failed to poll the sockets.
        ErrorAcceptSocket, ///< Failed to accept a new connection.
        ErrorReadSocket, ///< Failed to read from a socket.
        ErrorReplyCode, ///< The reply code dosnt exist.
    };

    /**
     * @class MyFtpErrors
     * @brief Custom exception class for the FTP server.
     *
     * Each error type has its own message that explains what went wrong.
     * You just create it with a error type and it will automaticly
     * set the right message for you.
     */
    class MyFtpErrors : public std::exception {
        std::map<MyFtpErrorType, std::string> _errorMap = {
            {IncorrectNumberArgs, "Incorrect number of arguments!"},
            {IncorrectArgs, "Incorrect arguments!"},
            {IncorrectPort, "Incorrect port!"},
            {IncorrectPath, "Incorrect path!"},
            {ErrorCreateSocket, "An error occurred while creating the server socket!"},
            {ErrorBindSocket, "An error occurred while binding the server socket!"},
            {ErrorBindSocket, "An error occurred while setting the server socket in listen state!"},
            {ErrorPollSocket, "An error occurred while polling the server socket!"},
            {ErrorAcceptSocket, "An error occurred while accepting connection from the server socket!"},
            {ErrorReadSocket, "An error occurred while reading a socket!"},
            {ErrorReplyCode, "This reply code doesn't exist!"},
        };

        std::string _errorMessage;

    public:
        /**
         * @brief Creates a new error with the given type.
         *
         * The error message is automaticly set based on the type.
         *
         * @param type The type of error that happend.
         */
        explicit MyFtpErrors(MyFtpErrorType type);

        /**
         * @brief Returns the error message as a C string.
         * @return A pointer to the error message.
         */
        [[nodiscard]] const char* what() const noexcept override;
    };
}
