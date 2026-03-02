//
// Created by pavel on 24/02/2026.
//

#include <utility>

#include "Client.h++"

#include "Errors/MyFtpErrors.h++"

namespace MyFtp {
    Client::Client(const int fd, std::unique_ptr<FtpSession> session, const std::string& rootPath) :
        _session(std::move(session)), _currentPath(rootPath), _activeModeSettings() {
        this->_fd = fd;
        this->_rootPath = std::filesystem::canonical(rootPath);
    }

    void Client::disconnect() {
        this->_mustLogOff = true;
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

    std::string Client::getVirtualPath() const {
        const auto relative = std::filesystem::relative(this->_currentPath, this->_rootPath);

        if (relative == ".")
            return "/";
        return "/" + relative.string();
    }

    void Client::sendReply(const replyCode code, const std::optional<std::string>& _customMessage) {
        if (_customMessage != std::nullopt) {
            this->_session->getOutputBuffer().append(*_customMessage);
            return;
        }
        if (!_replyMessage.contains(code))
            throw MyFtpErrors(ErrorReplyCode);
        this->_session->getOutputBuffer().append(_replyMessage[code]);
    }

    void Client::setCurrentPath(const std::filesystem::path& path) {
        this->_currentPath = path;
    }

    Socket& Client::getDataTransferSocket() {
        return this->_dataSocket;
    }

    dataTransferMode Client::getDataTransferMode() const {
        return this->_mode;
    }

    void Client::setDataTransferMode(const dataTransferMode mode) {
        this->_mode = mode;
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

    Client::ReadResult Client::readIncoming() {
        char buffer[4096] = {};
        const ssize_t bytesRead = this->_session->getControlSocket().read(buffer, sizeof(buffer) - 1);

        if (bytesRead > 0) {
            this->getSession()->getCommandBuffer().append(buffer, bytesRead);
            return Ok;
        }
        if (bytesRead == 0) {
            return Disconnected;
        }
        return Error;
    }

    std::optional<std::string> Client::nextCommand() const {
        const size_t pos = this->_session->getCommandBuffer().find("\r\n");
        std::string command = {};

        if (pos == std::string::npos)
            return std::nullopt;

        command = this->_session->getCommandBuffer().substr(0, pos);
        this->_session->getCommandBuffer().erase(0, pos + 2);
        return command;
    }

    void Client::flushOutput() {
        std::string& outputBuffer = this->getSession()->getOutputBuffer();
        if (outputBuffer.empty())
            return;

        this->_session->getControlSocket().write(outputBuffer.data(), outputBuffer.size());

        outputBuffer = "";
    }

    void Client::setActiveModeSetting(const std::string& ip, const unsigned short port) {
        this->_activeModeSettings = {
            .ip = ip,
            .port = port,
        };
    }
}
