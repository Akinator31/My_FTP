//
// Created by pavel on 01/03/2026.
//

#pragma once
#include <map>
#include <vector>
#include <sys/poll.h>

namespace MyFtp {
    /**
     * @class Poller
     * @brief A wrapper arround poll() that makes it easier to manage multiple file descriptors.
     *
     * This class lets you add and remove file descriptors and then check
     * if they are ready for reading, writing, or if they have errors.
     * Its basicly a nicer way to use the poll() system call.
     */
    class Poller {
        std::vector<pollfd> _pfds;
        std::map<int, size_t> _fdIndex;

        /**
         * @brief Gets the revents field of a file descriptor.
         * @param fd The file descriptor you want to check.
         * @return The revents value, or 0 if the fd is not in the poller.
         */
        short _reventOf(int fd);

    public:
        Poller() = default;

        /**
         * @brief Adds a new file descriptor to the poller with the given events.
         *
         * If the file descriptor is already in the poller, this function does nothing.
         *
         * @param fd The file descriptor you want to monitor.
         * @param events The events you want to watch for (like POLLIN or POLLOUT).
         */
        void add(int fd, short events);

        /**
         * @brief Removes a file descriptor from the poller.
         *
         * If the file descriptor is not in the poller, this function does nothing.
         *
         * @param fd The file descriptor you want to stop monitoring.
         */
        void remove(int fd);

        /**
         * @brief Waits until one of the monitored file descriptors is ready.
         * @param timeout How many milliseconds to wait. -1 means wait forever (default).
         * @return The number of file descriptors that are ready, or -1 on error.
         */
        int wait(int timeout = -1);

        /**
         * @brief Checks if a file descriptor has data ready to be readed.
         * @param fd The file descriptor to check.
         * @return true if there is data to read, false if not.
         */
        [[nodiscard]] bool isReadable(int fd);

        /**
         * @brief Checks if a file descriptor is ready to be writed to.
         * @param fd The file descriptor to check.
         * @return true if you can write to it, false if not.
         */
        [[nodiscard]] bool isWritable(int fd);

        /**
         * @brief Checks if a file descriptor is invalid (not open or bad).
         * @param fd The file descriptor to check.
         * @return true if the fd is invalid, false if its ok.
         */
        [[nodiscard]] bool isInvalid(int fd);

        /**
         * @brief Checks if a file descriptor has a error.
         * @param fd The file descriptor to check.
         * @return true if there is a error, false if not.
         */
        [[nodiscard]] bool hasError(int fd);

        /**
         * @brief Checks if the other side has hanged up (closed the connection).
         * @param fd The file descriptor to check.
         * @return true if the connection was closed by the other side, false if not.
         */
        [[nodiscard]] bool hasHangup(int fd);
    };
}
