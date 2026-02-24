//
// Created by pavel on 23/02/2026.
//

#pragma once
#include <map>
#include <string>
#include <netinet/in.h>

namespace MyFtp {
    enum FtpSessionType {
        FTPServer,
        FTPClient,
    };

    class FtpSession {
        int _controlSocket;
        int _dataSocket;
        sockaddr_in _socketConfiguration{};
        FtpSessionType _sessionType;
        std::string _commandBuffer;
        std::string _outputBuffer;

        static const std::map<int, std::string> replyCode;

    public:
        FtpSession(FtpSessionType type, int controlSocket);

        [[nodiscard]] int getControlSocket() const;
        sockaddr_in& getSocketConfiguration();
        std::string& getCommandBuffer();
        std::string& getOutputBuffer();

        void setSocketConfiguration(const sockaddr_in& config);
    };
}
