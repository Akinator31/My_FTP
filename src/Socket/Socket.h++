//
// Created by pavel on 26/02/2026.
//

#pragma once
#include <optional>
#include <string>
#include <netinet/in.h>

namespace MyFtp {
    /**
     * @class Socket
     * @brief A wrapper arround POSIX sockets that handle the lifecycle automaticly (RAII).
     *
     * This class is responsable for creating, binding, listening and accepting connections
     * on a TCP socket. It also handles reading and writing data trough the socket.
     * The socket is automaticly closed when the object is destroyed.
     *
     * @note Copy is disabled, only move is allowed.
     */
    class Socket {
        int _fd;
        sockaddr_in _socketConfig{};

    public:
        /**
         * @brief Creates a new TCP socket.
         * @throws MyFtpErrors if the socket creation failed.
         */
        Socket();

        /**
         * @brief Wraps a existing file descriptor into a Socket object.
         * @param socket The file descriptor of the socket you want to wrap.
         */
        explicit Socket(int socket);

        /**
         * @brief Destroys the socket and close the file descriptor if its still open.
         */
        ~Socket();

        /**
         * @brief Move constructor. Takes the ownership of the other socket.
         * @param other The socket your moving from. It will be invalidated after this.
         */
        Socket(Socket&& other) noexcept;

        /**
         * @brief Move assignement operator. Close the current socket and take the other one.
         * @param other The socket your moving from.
         * @return A reference to this socket.
         */
        Socket& operator=(Socket&& other) noexcept;

        Socket(const Socket&) = delete;
        Socket& operator=(const Socket&) = delete;

        /**
         * @brief Binds the socket to a specific port on all network interfaces.
         * @param port The port number you want to bind to.
         * @param socketConfigOpt The socket config if you want to use yours
         * @throws MyFtpErrors if the bind failed.
         */
        void bind(uint16_t port, const std::optional<sockaddr_in>& socketConfigOpt = std::nullopt);

        /**
         * @brief Puts the socket in a listening state so it can accept incomming connections.
         * @throws MyFtpErrors if listen failed.
         */
        void listen() const;

        /**
         * Establish a connection to a remote
         * @return The return of the connect function
         */
        int connect(const std::string& ip, unsigned short port) const;

        /**
         * @brief Accepts a new incomming connection and returns a new Socket for it.
         * @return A new Socket object that represents the client connection.
         * @throws MyFtpErrors if accept failed.
         */
        [[nodiscard]] Socket accept() const;

        /**
         * @brief Reads data from the socket into a buffer.
         * @param buffer Where the readed data will be stored.
         * @param size The maximum number of bytes to read.
         * @return The number of bytes readed, 0 if the connection was closed, or -1 on error.
         */
        [[nodiscard]] ssize_t read(void* buffer, size_t size) const;

        /**
         * @brief Writes data from a buffer to the socket.
         * @param buffer The data you want to send.
         * @param size The number of bytes to write.
         * @return The number of bytes writed, or -1 on error.
         */
        ssize_t write(const char* buffer, size_t size) const;

        /**
         * @brief Closes the socket if it was not already closed.
         */
        void close();

        /**
         * @brief Gets the file descriptor of the socket.
         * @return The file descriptor, or -1 if the socket is closed.
         */
        [[nodiscard]] int fd() const;

        /**
         * @brief Gets the sockaddr_in struct of the socket.
         * @return The sockaddr_in.
         */
        [[nodiscard]] sockaddr_in& getSin();
    };
}
