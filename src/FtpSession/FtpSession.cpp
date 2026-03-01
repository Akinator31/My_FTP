//
// Created by pavel on 23/02/2026.
//

#include "FtpSession.h++"

#include "Errors/MyFtpErrors.h++"

namespace MyFtp {
    FtpSession::FtpSession(const FtpSessionType type, Socket&& controlSocket) {
        if (controlSocket.fd() == -1)
            throw MyFtpErrors(ErrorCreateSocket);

        this->_controlSocket = std::move(controlSocket);
        this->_commandBuffer = "";
        this->_outputBuffer = {};
        this->_sessionType = type;
        this->_dataSocket = Socket(-1);
    }

    Socket& FtpSession::getControlSocket() {
        return this->_controlSocket;
    }

    std::string& FtpSession::getCommandBuffer() {
        return this->_commandBuffer;
    }

    std::string& FtpSession::getOutputBuffer() {
        return this->_outputBuffer;
    }
}
