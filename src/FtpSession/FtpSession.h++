//
// Created by pavel on 23/02/2026.
//

#pragma once
#include <string>
#include <netinet/in.h>

namespace my_ftp {
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

    public:
        FtpSession(FtpSessionType type, int controlSocket);

        [[nodiscard]] int getControlSocket() const;
        sockaddr_in& getSocketConfiguration();
        std::string& getCommandBuffer();

        void setSocketConfiguration(const sockaddr_in& config);
    };
}
