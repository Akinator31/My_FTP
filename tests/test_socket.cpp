#include <criterion/criterion.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <chrono>
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

Test(Socket, get_sin) {
    MyFtp::Socket sock;
    sock.bind(0);
    sockaddr_in& sin = sock.getSin();
    cr_assert_eq(sin.sin_family, AF_INET);
    cr_assert_eq(sin.sin_addr.s_addr, INADDR_ANY);
}

Test(Socket, bind_with_custom_config) {
    MyFtp::Socket sock;
    sockaddr_in config{};
    config.sin_family = AF_INET;
    config.sin_port = htons(0);
    config.sin_addr.s_addr = INADDR_ANY;
    sock.bind(0, config);
    sock.listen();
    cr_assert_geq(sock.fd(), 0);
}

Test(Socket, connect_success) {
    // Create a listening socket
    MyFtp::Socket server;
    server.bind(0);
    server.listen();

    // Get the port
    sockaddr_in sin{};
    socklen_t len = sizeof(sin);
    getsockname(server.fd(), reinterpret_cast<sockaddr*>(&sin), &len);
    unsigned short port = ntohs(sin.sin_port);

    // Connect to it
    MyFtp::Socket client;
    int ret = client.connect("127.0.0.1", port);
    cr_assert_eq(ret, 0);
}

Test(Socket, connect_failure) {
    MyFtp::Socket sock;
    // Connect to a port that is not listening
    int ret = sock.connect("127.0.0.1", 1);
    cr_assert_eq(ret, -1);
}

Test(Socket, accept_success) {
    MyFtp::Socket server;
    server.bind(0);
    server.listen();

    sockaddr_in sin{};
    socklen_t len = sizeof(sin);
    getsockname(server.fd(), reinterpret_cast<sockaddr*>(&sin), &len);
    unsigned short port = ntohs(sin.sin_port);

    // Connect from a raw socket
    int clientFd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    ::connect(clientFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

    MyFtp::Socket accepted = server.accept();
    cr_assert_geq(accepted.fd(), 0);
    close(clientFd);
}
