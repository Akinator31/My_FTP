//
// Created by pavel on 24/02/2026.
//

#include <utility>

#include "Client.h++"

#include "Errors/MyFtpErrors.h++"

namespace MyFtp {
    Client::Client(const pollfd pfd, std::unique_ptr<FtpSession> session, const std::string& rootPath) :
        _pfd(pfd), _session(std::move(session)), _currentPath(rootPath) {
        this->_rootPath = std::filesystem::canonical(rootPath);
    }

    void Client::disconnect() {
        this->_mustLogOff = true;
    }

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

    std::filesystem::path& Client::getRootPath() {
        return this->_rootPath;
    }

    std::filesystem::path& Client::getCurrentPath() {
        return this->_currentPath;
    }

    void Client::sendReply(const replyCode code) {
        if (!_replyMessage.contains(code))
            throw MyFtpErrors(ErrorReplyCode);
        this->_session->getOutputBuffer() = _replyMessage[code];
    }

    void Client::setCurrentPath(const std::filesystem::path& path) {
        this->_currentPath = path;
    }

    void Client::userLoggedIn() {
        this->_isClientLoggedIn = true;
    }

    bool Client::isClientAlreadyLoggedIn() const {
        return this->_isClientLoggedIn;
    }

    bool Client::mustLogOff() const {
        return this->_mustLogOff;
    }
}
