//
// Created by pavel on 05/03/2026.
//

#include "../Client/DataTransferManager.h++"

#include <format>
#include <fstream>
#include <iostream>
#include <ostream>
#include <sys/wait.h>

#include "Utils/Utils.h++"

namespace MyFtp {
    DataTransferManager::DataTransferManager(Socket& controlSocket) : _activeModeSettings() {
        this->_controlSocket = &controlSocket;
    }

    void DataTransferManager::setPassiveMode() {
        if (this->_mode == UNKNOWN) {
            this->_mode = AWAITING_PASSIVE_CONNECTION;
            this->_dataSocket = Socket();
            this->_dataSocket.bind(0);
            this->_dataSocket.listen();
            return;
        }
        if (this->_mode == AWAITING_PASSIVE_CONNECTION)
            this->_mode = PASSIVE;
    }

    void DataTransferManager::setActiveMode(const std::string& ip, const unsigned short port) {
        this->_mode = ACTIVE;
        this->_activeModeSettings = {
            .ip = ip,
            .port = port,
        };
    }

    void DataTransferManager::setTransferContext(const TransferContext& context) {
        this->_transferContext = std::make_unique<TransferContext>(context);
    }

    bool DataTransferManager::isMode(const dataTransferMode mode) const {
        return this->_mode == mode;
    }

    void DataTransferManager::checkCurrentTransfer() const {
        if (this->_transferContext != nullptr) {
            this->_transferContext->response150sent = true;
        }
    }

    Socket& DataTransferManager::getDataSocket() {
        return this->_dataSocket;
    }

    void DataTransferManager::acceptPassiveConnection() {
        Socket passiveDataSocket = this->_dataSocket.accept();
        this->_dataSocket = std::move(passiveDataSocket);
        this->_mode = PASSIVE;
    }

    std::string DataTransferManager::formatPasvResponse() const {
        unsigned char* ip;
        sockaddr_in sin{};
        socklen_t sin_size = sizeof(sin);

        getsockname(this->_dataSocket.fd(), reinterpret_cast<sockaddr*>(&sin),
                    &sin_size);
        const unsigned short port = ntohs(sin.sin_port);

        if (sin.sin_addr.s_addr == INADDR_ANY) {
            sockaddr_in& clientSin = this->_controlSocket->getSin();
            ip = reinterpret_cast<unsigned char*>(&clientSin.sin_addr.s_addr);
        } else {
            ip = reinterpret_cast<unsigned char*>(&sin.sin_addr.s_addr);
        }

        const unsigned char p1 = port / 256;
        const unsigned char p2 = port % 256;

        std::string result = std::format("227 Entering Passive Mode ({}, {}, {}, {}, {}, {})\r\n", ip[0], ip[1], ip[2],
                                         ip[3], p1, p2);

        return result;
    }

    dataTransferMode DataTransferManager::handleDataTransferCommand() {
        const int childPid = fork();

        if (childPid == -1)
            return ERROR;

        if (childPid == 0) {
            if (this->_transferContext->type == LIST) {
                const std::string listResult = Utils::getOutputCommand(
                    "/bin/ls -l " + this->_transferContext->directory);
                [[maybe_unused]] ssize_t readBytes = this->_dataSocket.write(listResult.c_str(), listResult.size());
            }
            if (this->_transferContext->type == RETR) {
                const size_t filesize = std::filesystem::file_size(this->_transferContext->filename);
                std::vector<char> buffer(filesize);

                std::ifstream file(this->_transferContext->filename, std::ios::binary);
                file.read(buffer.data(), static_cast<std::streamsize>(filesize));

                [[maybe_unused]] ssize_t readBytes = this->_dataSocket.write(buffer.data(), buffer.size());
                file.close();
            }
            if (this->_transferContext->type == STOR) {
                std::ofstream file(this->_transferContext->filename, std::ios::binary | std::ios::trunc);
                std::vector<char> buffer(1024);
                ssize_t bytesRead;

                while ((bytesRead = this->_dataSocket.read(buffer.data(), 1024)) > 0) {
                    file.write(buffer.data(), bytesRead);
                }
                file.close();
            }
            exit(0);
        }
        this->_dataSocket.close();
        this->_dataTransferChildPid = childPid;
        this->_transferContext->response150sent = false;
        return SENT;
    }

    dataTransferMode DataTransferManager::updateDataTransfer() {
        if (this->_transferContext != nullptr && this->_transferContext->response150sent == true) {
            if (this->_mode == ACTIVE) {
                Socket socket;
                if (socket.connect(this->_activeModeSettings.ip, this->_activeModeSettings.port) == -1) {
                    this->_transferContext.reset();
                    return ERROR;
                }
                this->_dataSocket = std::move(socket);
            }
            return this->handleDataTransferCommand();
        }

        if (this->_dataTransferChildPid > 0) {
            int status;

            if (waitpid(this->_dataTransferChildPid, &status, WNOHANG) > 0) {
                this->_transferContext.reset();
                this->_dataTransferChildPid = -1;
                this->_mode = UNKNOWN;
                return CLOSING;
            }
        }
        return NOTHING;
    }
}
