//
// Created by pavel on 23/02/2026.
//

#pragma once
#include <map>
#include <string>

#include "Socket/Socket.h++"

namespace MyFtp {
    enum FtpSessionType {
        FTPServer,
        FTPClient,
    };

    class FtpSession {
        Socket _controlSocket;
        Socket _dataSocket;
        FtpSessionType _sessionType;
        std::string _commandBuffer;
        std::string _outputBuffer;

        static const std::map<int, std::string> replyCode;

    public:
        FtpSession(FtpSessionType type, Socket&& controlSocket);

        [[nodiscard]] Socket& getControlSocket();
        std::string& getCommandBuffer();
        std::string& getOutputBuffer();
    };
}
