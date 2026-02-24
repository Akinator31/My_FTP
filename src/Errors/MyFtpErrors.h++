//
// Created by pavel on 19/02/2026.
//

#pragma once
#include <exception>
#include <string>
#include <map>

namespace MyFtp {
    enum MyFtpErrorType {
        IncorrectNumberArgs,
        IncorrectArgs,
        IncorrectPort,
        IncorrectPath,
        ErrorCreateSocket,
        ErrorBindSocket,
        ErrorListenSocket,
        ErrorPollSocket,
        ErrorAcceptSocket,
        ErrorReadSocket,
    };

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
            {ErrorReadSocket, "An error occurred while reading a socket!"}
        };

        std::string _errorMessage;

    public:
        explicit MyFtpErrors(MyFtpErrorType type);
        [[nodiscard]] const char* what() const noexcept override;
    };
}
