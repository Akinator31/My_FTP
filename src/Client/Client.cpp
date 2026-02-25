//
// Created by pavel on 24/02/2026.
//

#include <utility>

#include "Client.h++"

#include "Errors/MyFtpErrors.h++"

namespace MyFtp {
    Client::Client(const pollfd pfd, std::unique_ptr<FtpSession> session) : _pfd(pfd), _session(std::move(session)) {}

    pollfd& Client::getPfd() {
        return this->_pfd;
    }

    std::unique_ptr<FtpSession>& Client::getSession() {
        return this->_session;
    }

    std::string& Client::getUsername() {
        return this->_username;
    }

    std::string& Client::getPassword() {
        return this->_password;
    }

    void Client::sendReply(const int replyCode) {
        if (!_replyMessage.contains(replyCode))
            throw MyFtpErrors(ErrorReplyCode);
        this->_session->getOutputBuffer() = _replyMessage[replyCode];
    }
}
