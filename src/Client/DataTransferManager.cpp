//
// Created by pavel on 05/03/2026.
//

#include "../Client/DataTransferManager.h++"

#include <format>
#include <iostream>
#include <ostream>
#include <sys/wait.h>

namespace MyFtp {
    DataTransferManager::DataTransferManager(Socket& controlSocket) : _activeModeSettings() {
        std::cout << "SOCKET : " << controlSocket.fd() << std::endl;
        this->_controlSocket = &controlSocket;
    }

    void DataTransferManager::setPassiveMode() {
        if (this->_mode == UNKNOWN) {
            this->_mode = AWAITING_PASSIVE_CONNECTION;
            this->_dataSocket = Socket();
            this->_dataSocket.bind(0);
            this->_dataSocket.listen();
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
            std::cout << "150 response sent !" << std::endl;
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

    dataTransferMode DataTransferManager::updateDataTransfer() {
        if (this->_transferContext != nullptr && this->_transferContext->response150sent == true) {
            std::cout << "SENDING DATA !" << std::endl;
            const int childPid = fork(); // gérer fork return -1;
            if (childPid == 0) {
                [[maybe_unused]] ssize_t readBytes = this->_dataSocket.write("BONJOUR\r\n", 9);
                exit(0);
            }
            this->_dataSocket.close();
            this->_dataTransferChildPid = childPid;
            this->_transferContext->response150sent = false;
            return SENT;
        }

        if (this->_dataTransferChildPid > 0) {
            int status;

            if (waitpid(this->_dataTransferChildPid, &status, WNOHANG) > 0) {
                if (WIFEXITED(status))
                    std::cout << "CHILD EXITED WITH STATUS : " << WEXITSTATUS(status) << std::endl;
                this->_transferContext.reset();
                this->_dataTransferChildPid = -1;
                this->_mode = UNKNOWN;
                return CLOSING;
            }
        }
        return NOTHING;
    }
}
