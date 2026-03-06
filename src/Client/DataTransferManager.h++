//
// Created by pavel on 05/03/2026.
//

#pragma once
#include <memory>
#include <string>

#include "Socket/Socket.h++"

namespace MyFtp {
    class Client;

    enum TransferType {
        NONE, LIST, RETR, STOR,
    };

    struct TransferContext {
        TransferType type;
        std::string filename;
        std::string directory;
        bool response150sent;
    };

    /**
     * @enum dataTransferMode
     * @brief All the FTP data transfer mode.
     *
     * Each value correspond to a standard FTP data transfer mode from the RFC 959.
     */
    enum dataTransferMode {
        UNKNOWN,
        PASSIVE,
        ACTIVE,
        AWAITING_PASSIVE_CONNECTION,
        CLOSING,
        SENT,
        NOTHING,
        ERROR,
    };

    struct activeTransferModeSettings {
        std::string ip;
        unsigned short port;
    };

    class DataTransferManager {
        Socket _dataSocket;
        Socket* _controlSocket;

        int _dataTransferChildPid = -1;
        dataTransferMode _mode = UNKNOWN;
        activeTransferModeSettings _activeModeSettings;
        std::unique_ptr<TransferContext> _transferContext = nullptr;

    public:
        explicit DataTransferManager(Socket& controlSocket);

        void setPassiveMode();
        void setActiveMode(const std::string& ip, unsigned short port);

        /**
         * Format the PASV response from a sockaddr_in struct
         * @return The formatted string for the pasv command.
         */
        [[nodiscard]] std::string formatPasvResponse() const;
        [[nodiscard]] bool isMode(dataTransferMode mode) const;
        void setTransferContext(const TransferContext& context);
        Socket& getDataSocket();

        void acceptPassiveConnection();
        dataTransferMode updateDataTransfer();
        void checkCurrentTransfer() const;
    };
}
