//
// Created by pavel on 23/02/2026.
//

#pragma once
#include <map>
#include <string>

#include "Socket/Socket.h++"

namespace MyFtp {
    /**
     * @enum FtpSessionType
     * @brief The type of a FTP session. It can be the server or a client.
     */
    enum FtpSessionType {
        FTPServer, ///< This session belongs to the server.
        FTPClient, ///< This session belongs to a client.
    };

    /**
     * @class FtpSession
     * @brief Manages a FTP session with its control socket, data socket and buffers.
     *
     * A FtpSession holds the sockets that are used for comunication and also
     * the buffers where the incomming commands and outgoing replys are stored
     * before they get processed or sended.
     */
    class FtpSession {
        Socket _controlSocket;
        Socket _dataSocket;
        FtpSessionType _sessionType;
        std::string _commandBuffer;
        std::string _outputBuffer;

        static const std::map<int, std::string> replyCode;

    public:
        /**
         * @brief Creates a new FTP session with a type and a control socket.
         * @param type The type of the session (FTPServer or FTPClient).
         * @param controlSocket The socket used for sending commands and replys. It will be moved.
         * @throws MyFtpErrors if the control socket file descriptor is invalid (-1).
         */
        FtpSession(FtpSessionType type, Socket&& controlSocket);

        /**
         * @brief Gets the control socket of this session.
         * @return A reference to the control Socket object.
         */
        [[nodiscard]] Socket& getControlSocket();

        /**
         * @brief Gets the buffer where incomming commands are stored.
         *
         * Commands are appended here when data is readed from the socket.
         * They stay in the buffer until they are parsed and removed.
         *
         * @return A reference to the command buffer string.
         */
        std::string& getCommandBuffer();

        /**
         * @brief Gets the buffer where outgoing replys are waiting to be sended.
         *
         * Reply messages are appended here and then flushed to the socket
         * when the socket is ready to write.
         *
         * @return A reference to the output buffer string.
         */
        std::string& getOutputBuffer();
    };
}
