/**
 * @file DataTransferManager.h++
 * @brief Data transfer management for FTP clients (active and passive modes).
 * @date 05/03/2026
 * @author pavel
 */

#pragma once
#include <memory>
#include <string>

#include "Socket/Socket.h++"

namespace MyFtp {
    class Client;

    /**
     * @enum TransferType
     * @brief The type of data transfer requested by the client.
     */
    enum TransferType {
        NONE, ///< No transfer in progress.
        LIST, ///< Directory listing transfer.
        RETR, ///< File retrieval (download) transfer.
        STOR, ///< File storage (upload) transfer.
    };

    /**
     * @struct TransferContext
     * @brief Holds the context of an ongoing data transfer.
     *
     * This structure stores all the information needed to execute
     * a data transfer command (LIST, RETR or STOR) in a child process.
     */
    struct TransferContext {
        TransferType type; ///< The type of the transfer (LIST, RETR, STOR).
        std::string filename; ///< The file path for RETR/STOR operations.
        std::string directory; ///< The directory path for LIST operations.
        bool response150sent; ///< Whether the "150" preliminary reply has been sent.
    };

    /**
     * @enum dataTransferMode
     * @brief All the FTP data transfer modes.
     *
     * Each value corresponds to a state in the data transfer lifecycle
     * as defined in RFC 959.
     */
    enum dataTransferMode {
        UNKNOWN, ///< No data transfer mode has been set yet.
        PASSIVE, ///< Passive mode connection established.
        ACTIVE, ///< Active mode configured, ready to connect.
        AWAITING_PASSIVE_CONNECTION, ///< Server is listening, waiting for the client to connect.
        CLOSING, ///< The data connection is being closed.
        SENT, ///< The data has been sent to the child process.
        NOTHING, ///< No action needed at this time.
        ERROR, ///< An error occurred during data transfer.
    };

    /**
     * @struct activeTransferModeSettings
     * @brief Stores the IP address and port for an active mode data connection.
     *
     * These values are parsed from the PORT command sent by the client.
     */
    struct activeTransferModeSettings {
        std::string ip; ///< The IP address to connect to in active mode.
        unsigned short port; ///< The port number to connect to in active mode.
    };

    /**
     * @class DataTransferManager
     * @brief Manages FTP data connections for a single client.
     *
     * This class handles both active and passive data transfer modes.
     * It creates child processes via fork() to perform the actual data
     * transfer (LIST, RETR, STOR) without blocking the main server loop.
     */
    class DataTransferManager {
        Socket _dataSocket; ///< The socket used for data transfer.
        Socket* _controlSocket; ///< Pointer to the client's control socket.

        int _dataTransferChildPid = -1; ///< PID of the child process handling the transfer (-1 if none).
        dataTransferMode _mode = UNKNOWN; ///< The current data transfer mode.
        activeTransferModeSettings _activeModeSettings; ///< Settings for active mode (IP and port).
        std::unique_ptr<TransferContext> _transferContext = nullptr;
        ///< The current transfer context (or nullptr if none).

        /**
         * @brief Forks a child process to handle the data transfer command.
         *
         * Depending on the TransferContext type, the child process will
         * execute a LIST, RETR or STOR operation on the data socket.
         *
         * @return The resulting data transfer mode (SENT on success, ERROR on failure).
         */
        dataTransferMode handleDataTransferCommand();

    public:
        /**
         * @brief Constructs a DataTransferManager linked to a control socket.
         * @param controlSocket A reference to the client's control socket.
         */
        explicit DataTransferManager(Socket& controlSocket);

        /**
         * @brief Switches the manager to passive mode.
         *
         * On the first call, creates a new data socket, binds it to a random port
         * and starts listening. On the second call, transitions from
         * AWAITING_PASSIVE_CONNECTION to PASSIVE.
         */
        void setPassiveMode();

        /**
         * @brief Switches the manager to active mode with the given address.
         * @param ip The IP address to connect to when the transfer starts.
         * @param port The port number to connect to when the transfer starts.
         */
        void setActiveMode(const std::string& ip, unsigned short port);

        /**
         * @brief Formats the PASV response string from the data socket address.
         *
         * Builds the "227 Entering Passive Mode (h1,h2,h3,h4,p1,p2)" reply
         * using the address and port of the bound data socket.
         *
         * @return The formatted PASV response string ready to be sent.
         */
        [[nodiscard]] std::string formatPasvResponse() const;

        /**
         * @brief Checks if the manager is currently in the given mode.
         * @param mode The data transfer mode to compare against.
         * @return true if the current mode matches, false otherwise.
         */
        [[nodiscard]] bool isMode(dataTransferMode mode) const;

        /**
         * @brief Sets the transfer context for the next data transfer.
         * @param context The TransferContext describing the operation to perform.
         */
        void setTransferContext(const TransferContext& context);

        /**
         * @brief Gets the data socket used for transfers.
         * @return A reference to the data Socket object.
         */
        Socket& getDataSocket();

        /**
         * @brief Accepts an incoming passive mode connection on the data socket.
         *
         * Replaces the listening data socket with the newly accepted connection
         * and transitions the mode to PASSIVE.
         */
        void acceptPassiveConnection();

        /**
         * @brief Updates the state of the current data transfer.
         *
         * If a transfer context is ready and the 150 reply has been sent,
         * it initiates the transfer (connecting in active mode if needed).
         * It also checks if a running child process has finished.
         *
         * @return The current state of the transfer (CLOSING, SENT, ERROR, or NOTHING).
         */
        dataTransferMode updateDataTransfer();

        /**
         * @brief Marks the current transfer context as having its 150 reply sent.
         *
         * This is called after the "150 File status okay" reply is queued,
         * signaling that the actual data transfer can begin.
         */
        void checkCurrentTransfer() const;
    };
}
