//
// Created by pavel on 01/03/2026.
//

#include "Poller.h++"

#include <iostream>

#include "Client/Client.h++"

namespace MyFtp {
    short Poller::_reventOf(const int fd) {
        if (!this->_fdIndex.contains(fd))
            return 0;
        return this->_pfds[this->_fdIndex[fd]].revents;
    }

    void Poller::add(const int fd, const short events) {
        if (this->_fdIndex.contains(fd))
            return;
        this->_fdIndex[fd] = this->_pfds.size();
        this->_pfds.push_back({
            .fd = fd,
            .events = events,
            .revents = 0
        });
    }

    void Poller::remove(const int fd) {
        const auto it = this->_fdIndex.find(fd);
        if (it == this->_fdIndex.end())
            return;

        const size_t idx = it->second;
        size_t last = this->_pfds.size() - 1;

        if (idx != last) {
            std::swap(this->_pfds[idx], this->_pfds[last]);
            this->_fdIndex[this->_pfds[idx].fd] = idx;
        }
        this->_pfds.pop_back();
        this->_fdIndex.erase(it);
    }

    int Poller::wait(const int timeout) {
        return poll(this->_pfds.data(), this->_pfds.size(), timeout);
    }

    bool Poller::isReadable(const int fd) {
        return this->_reventOf(fd) & POLLIN;
    }

    bool Poller::isWritable(const int fd) {
        return this->_reventOf(fd) & POLLOUT;
    }

    bool Poller::isInvalid(const int fd) {
        return this->_reventOf(fd) & POLLNVAL;
    }

    bool Poller::hasError(const int fd) {
        return this->_reventOf(fd) & POLLERR;
    }

    bool Poller::hasHangup(const int fd) {
        return this->_reventOf(fd) & POLLHUP;
    }

    void Poller::handleAwaitingDataConnection(Client& client) {
        Socket& dataTransferSocket = client.getDataTransferSocket();

        if (!this->_fdIndex.contains(dataTransferSocket.fd()))
            this->add(dataTransferSocket.fd(), POLLIN);

        if (this->isReadable(dataTransferSocket.fd()) && client.getDataTransferMode() == AWAITING_PASSIVE_CONNECTION) {
            Socket passiveDataSocket = dataTransferSocket.accept();
            dataTransferSocket = std::move(passiveDataSocket);
            client.setDataTransferMode(PASSIVE);
        }
    }
}

