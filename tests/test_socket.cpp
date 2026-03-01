#include <criterion/criterion.h>
#include <sys/socket.h>
#include <unistd.h>
#include "Socket/Socket.h++"
#include "Errors/MyFtpErrors.h++"

Test(Socket, create_default_socket) {
    MyFtp::Socket sock;
    cr_assert_geq(sock.fd(), 0);
}

Test(Socket, create_from_fd) {
    int fds[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    {
        MyFtp::Socket sock(fds[0]);
        cr_assert_eq(sock.fd(), fds[0]);
    }
    close(fds[1]);
}

Test(Socket, create_from_invalid_fd) {
    MyFtp::Socket sock(-1);
    cr_assert_eq(sock.fd(), -1);
}

Test(Socket, close_socket) {
    int fds[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    MyFtp::Socket sock(fds[0]);
    cr_assert_neq(sock.fd(), -1);
    sock.close();
    cr_assert_eq(sock.fd(), -1);
    close(fds[1]);
}

Test(Socket, close_already_closed) {
    MyFtp::Socket sock(-1);
    sock.close();
    cr_assert_eq(sock.fd(), -1);
}

Test(Socket, move_constructor) {
    int fds[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    MyFtp::Socket sock1(fds[0]);
    int originalFd = sock1.fd();
    MyFtp::Socket sock2(std::move(sock1));
    cr_assert_eq(sock2.fd(), originalFd);
    cr_assert_eq(sock1.fd(), -1);
    close(fds[1]);
}

Test(Socket, move_assignment) {
    int fds1[2];
    int fds2[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds1), 0);
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds2), 0);
    MyFtp::Socket sock1(fds1[0]);
    MyFtp::Socket sock2(fds2[0]);
    int fd2 = sock2.fd();
    sock1 = std::move(sock2);
    cr_assert_eq(sock1.fd(), fd2);
    cr_assert_eq(sock2.fd(), -1);
    close(fds1[1]);
    close(fds2[1]);
}

Test(Socket, move_assignment_self) {
    int fds[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    MyFtp::Socket sock(fds[0]);
    int originalFd = sock.fd();
    sock = std::move(sock);
    cr_assert_eq(sock.fd(), originalFd);
    close(fds[1]);
}

Test(Socket, read_write) {
    int fds[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    MyFtp::Socket writer(fds[0]);
    MyFtp::Socket reader(fds[1]);

    const char* msg = "hello";
    ssize_t written = writer.write(msg, 5);
    cr_assert_eq(written, 5);

    char buf[64] = {};
    ssize_t rd = reader.read(buf, sizeof(buf) - 1);
    cr_assert_eq(rd, 5);
    cr_assert_str_eq(buf, "hello");
}

Test(Socket, read_closed_connection) {
    int fds[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    MyFtp::Socket reader(fds[0]);
    close(fds[1]);

    char buf[64] = {};
    ssize_t rd = reader.read(buf, sizeof(buf) - 1);
    cr_assert_eq(rd, 0);
}

Test(Socket, bind_and_listen) {
    MyFtp::Socket sock;
    sock.bind(0);
    sock.listen();
    cr_assert_geq(sock.fd(), 0);
}

Test(Socket, bind_invalid_fd) {
    MyFtp::Socket sock(-1);
    try {
        sock.bind(9999);
        cr_assert_fail("Should have thrown");
    }
    catch (const MyFtp::MyFtpErrors& e) {
        cr_assert_not_null(e.what());
    }
}

Test(Socket, listen_invalid_fd) {
    MyFtp::Socket sock(-1);
    try {
        sock.listen();
        cr_assert_fail("Should have thrown");
    }
    catch (const MyFtp::MyFtpErrors& e) {
        cr_assert_not_null(e.what());
    }
}

Test(Socket, accept_no_connection) {
    MyFtp::Socket sock(-1);
    try {
        auto s = sock.accept();
        (void)s;
        cr_assert_fail("Should have thrown");
    }
    catch (const MyFtp::MyFtpErrors& e) {
        cr_assert_not_null(e.what());
    }
}

Test(Socket, fd_getter) {
    MyFtp::Socket sock(42);
    cr_assert_eq(sock.fd(), 42);
}

