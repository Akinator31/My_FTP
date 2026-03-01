//
// Created by pavel on 19/02/2026.
//

#pragma once
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "Client/Client.h++"
#include "FtpSession/FtpSession.h++"
#include "Poller/Poller.h++"

namespace MyFtp {
    /**
     * @class SignalHandler
     * @brief Handles UNIX signals for the server (like SIGINT when you press Ctrl+C).
     */
    class SignalHandler {
    public:
        static bool mustClose; ///< Set to true when the server needs to stop.

        /**
         * @brief The handler function that gets called when SIGINT is recieved.
         * @param code The signal code (not really used, but required by signal()).
         */
        static void sigintHandler(int code);
    };

    /**
     * @class Server
     * @brief The main FTP server class. It listens for connections and handles client commands.
     *
     * The server uses poll() to handle multiple clients at the same time.
     * When a client sends a command, the server looks it up in the command map
     * and calls the right handler function.
     */
    class Server {
        std::string _path;
        FtpSession _serverSession;

        std::vector<Client> _clients;
        std::map<std::string, std::function<void (Client&, const std::string&)>> _funcMap;

        Poller _poller;

        /**
         * @brief Accepts a new client connection and adds it to the client list.
         *
         * This creates a new Client object with its own FtpSession and
         * sends the "220 Service ready" reply to let the client know its connected.
         *
         * @throws MyFtpErrors if the accept() call fails.
         */
        void _acceptClientConnection();

        /**
         * @brief Disconnects a client and removes it from the client list.
         * @param clientIndex The index of the client in the vector. It will be decremented after removal.
         * @param needToClose If true, the socket will be closed manualy before removing.
         */
        void _disconnectClient(size_t& clientIndex, bool needToClose);

        /**
         * @brief Parses a raw command string and calls the right handler function.
         *
         * If the command is not found in the map, it sends a 500 error to the client.
         *
         * @param client The client who sended the command.
         * @param command The full command string (like "USER Anonymous" or "QUIT").
         */
        void _handleCommand(Client& client, const std::string& command);

    public:
        /**
         * @brief Creates the FTP server, binds it to a port and prepares the command map.
         * @param port The port number to listen on.
         * @param path The root directory for the FTP server.
         */
        Server(size_t port, const std::string& path);

        /**
         * @brief Starts the main server loop.
         *
         * The server will keep running until SIGINT is recieved (Ctrl+C).
         * It uses poll() to wait for events on all the sockets and then
         * handles new connections, client commands, and disconnections.
         *
         * @throws MyFtpErrors if poll() fails.
         */
        void start();
    };
}
