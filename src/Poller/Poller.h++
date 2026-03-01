//
// Created by pavel on 01/03/2026.
//

#pragma once
#include <map>
#include <vector>
#include <sys/poll.h>

namespace MyFtp {
    class Poller {
        std::vector<pollfd> _pfds;
        std::map<int, size_t> _fdIndex;

        short _reventOf(int fd);

    public:
        Poller() = default;
        void add(int fd, short events);
        void remove(int fd);

        int wait(int timeout = -1);

        [[nodiscard]] bool isReadable(int fd);
        [[nodiscard]] bool isWritable(int fd);
        [[nodiscard]] bool isInvalid(int fd);
        [[nodiscard]] bool hasError(int fd);
        [[nodiscard]] bool hasHangup(int fd);
    };
}
