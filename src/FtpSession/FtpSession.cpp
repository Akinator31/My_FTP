//
// Created by pavel on 23/02/2026.
//

#include "FtpSession.h++"

#include "Errors/MyFtpErrors.h++"

namespace MyFtp {
    FtpSession::FtpSession(const FtpSessionType type, const int controlSocket) {
        if (controlSocket == -1)
            throw MyFtpErrors(ErrorCreateSocket);

        this->_controlSocket = controlSocket;
        this->_commandBuffer = "";
        this->_outputBuffer = "";
        this->_sessionType = type;
        this->_dataSocket = -1;
    }

    int FtpSession::getControlSocket() const {
        return this->_controlSocket;
    }

    void FtpSession::setSocketConfiguration(const sockaddr_in& config) {
        this->_socketConfiguration = config;
    }

    sockaddr_in& FtpSession::getSocketConfiguration() {
        return this->_socketConfiguration;
    }

    std::string& FtpSession::getCommandBuffer() {
        return this->_commandBuffer;
    }

    std::string& FtpSession::getOutputBuffer() {
        return this->_outputBuffer;
    }
}
