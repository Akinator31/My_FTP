#include <criterion/criterion.h>
#include <thread>
#include <chrono>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include "Server/Server.h++"
#include "Errors/MyFtpErrors.h++"

// ========== Helpers ==========

static int connectToServer(uint16_t port, int retries = 30) {
    for (int i = 0; i < retries; i++) {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd == -1) return -1;

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");

        if (connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0)
            return fd;
        close(fd);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    return -1;
}

static std::string readReply(int fd, int timeout_ms = 500) {
    std::string result;
    char buf[1024];
    auto start = std::chrono::steady_clock::now();
    while (true) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start).count();
        if (elapsed > timeout_ms) break;

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        timeval tv{0, 50000};
        if (select(fd + 1, &fds, nullptr, nullptr, &tv) > 0) {
            ssize_t n = read(fd, buf, sizeof(buf) - 1);
            if (n <= 0) break;
            buf[n] = '\0';
            result += buf;
        }
    }
    return result;
}

static void sendCmd(int fd, const char* cmd) {
    std::string msg = std::string(cmd) + "\r\n";
    write(fd, msg.c_str(), msg.size());
}

static void stopServer(uint16_t port) {
    MyFtp::SignalHandler::mustClose = true;
    int dummy = connectToServer(port, 5);
    if (dummy >= 0) close(dummy);
}

// ========== SignalHandler ==========

Test(SignalHandler, must_close_default_false) {
    MyFtp::SignalHandler::mustClose = false;
    cr_assert_not(MyFtp::SignalHandler::mustClose);
}

Test(SignalHandler, sigint_handler_sets_must_close) {
    MyFtp::SignalHandler::mustClose = false;
    MyFtp::SignalHandler::sigintHandler(SIGINT);
    cr_assert(MyFtp::SignalHandler::mustClose);
    MyFtp::SignalHandler::mustClose = false;
}

// ========== Server constructor ==========

Test(Server, constructor_binds_and_listens) {
    MyFtp::Server server(0, "/tmp");
    (void)server;
}

// ========== Full session: USER + PASS + QUIT ==========

Test(Server, full_login_and_quit) {
    uint16_t port = 14200;
    MyFtp::SignalHandler::mustClose = false;

    std::thread t([port]() {
        try {
            MyFtp::Server server(port, "/tmp");
            server.start();
        }
        catch (...) {}
    });

    int fd = connectToServer(port);
    cr_assert_neq(fd, -1, "Could not connect");

    // 220 welcome
    std::string welcome = readReply(fd);
    cr_assert(welcome.find("220") != std::string::npos);

    // USER Anonymous -> 331
    sendCmd(fd, "USER Anonymous");
    std::string r1 = readReply(fd);
    cr_assert(r1.find("331") != std::string::npos);

    // PASS -> 230
    sendCmd(fd, "PASS");
    std::string r2 = readReply(fd);
    cr_assert(r2.find("230") != std::string::npos);

    // QUIT -> 221
    sendCmd(fd, "QUIT");
    std::string r3 = readReply(fd);
    cr_assert(r3.find("221") != std::string::npos);

    close(fd);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stopServer(port);
    t.join();
}

// ========== Unknown command -> 500 ==========

Test(Server, unknown_command_returns_500) {
    uint16_t port = 14201;
    MyFtp::SignalHandler::mustClose = false;

    std::thread t([port]() {
        try {
            MyFtp::Server server(port, "/tmp");
            server.start();
        }
        catch (...) {}
    });

    int fd = connectToServer(port);
    cr_assert_neq(fd, -1);
    readReply(fd); // 220

    sendCmd(fd, "FOOBAR");
    std::string r = readReply(fd);
    cr_assert(r.find("500") != std::string::npos);

    close(fd);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stopServer(port);
    t.join();
}

// ========== Client abrupt disconnect ==========

Test(Server, client_abrupt_disconnect) {
    uint16_t port = 14202;
    MyFtp::SignalHandler::mustClose = false;

    std::thread t([port]() {
        try {
            MyFtp::Server server(port, "/tmp");
            server.start();
        }
        catch (...) {}
    });

    int fd = connectToServer(port);
    cr_assert_neq(fd, -1);
    readReply(fd); // 220

    // close abruptly
    close(fd);

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    stopServer(port);
    t.join();
}

// ========== CWD and CDUP through server ==========

Test(Server, cwd_and_cdup_integration) {
    uint16_t port = 14203;
    MyFtp::SignalHandler::mustClose = false;

    std::thread t([port]() {
        try {
            MyFtp::Server server(port, "/tmp");
            server.start();
        }
        catch (...) {}
    });

    int fd = connectToServer(port);
    cr_assert_neq(fd, -1);
    readReply(fd); // 220

    // Login
    sendCmd(fd, "USER Anonymous");
    readReply(fd);
    sendCmd(fd, "PASS");
    readReply(fd);

    // CWD .
    sendCmd(fd, "CWD .");
    std::string r1 = readReply(fd);
    cr_assert(r1.find("250") != std::string::npos || r1.find("550") != std::string::npos);

    // CDUP
    sendCmd(fd, "CDUP");
    std::string r2 = readReply(fd);
    cr_assert(r2.find("200") != std::string::npos || r2.find("550") != std::string::npos);

    sendCmd(fd, "QUIT");
    readReply(fd);
    close(fd);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stopServer(port);
    t.join();
}

// ========== Multiple commands pipelined ==========

Test(Server, pipelined_commands) {
    uint16_t port = 14204;
    MyFtp::SignalHandler::mustClose = false;

    std::thread t([port]() {
        try {
            MyFtp::Server server(port, "/tmp");
            server.start();
        }
        catch (...) {}
    });

    int fd = connectToServer(port);
    cr_assert_neq(fd, -1);
    readReply(fd); // 220

    const char* cmds = "USER Anonymous\r\nPASS\r\nQUIT\r\n";
    write(fd, cmds, strlen(cmds));

    std::string r = readReply(fd, 1000);
    cr_assert(r.find("331") != std::string::npos);
    cr_assert(r.find("230") != std::string::npos);
    cr_assert(r.find("221") != std::string::npos);

    close(fd);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stopServer(port);
    t.join();
}

// ========== Multiple clients simultaneously ==========

Test(Server, multiple_clients) {
    uint16_t port = 14205;
    MyFtp::SignalHandler::mustClose = false;

    std::thread t([port]() {
        try {
            MyFtp::Server server(port, "/tmp");
            server.start();
        }
        catch (...) {}
    });

    int fd1 = connectToServer(port);
    cr_assert_neq(fd1, -1);
    std::string w1 = readReply(fd1);
    cr_assert(w1.find("220") != std::string::npos);

    int fd2 = connectToServer(port);
    cr_assert_neq(fd2, -1);
    std::string w2 = readReply(fd2);
    cr_assert(w2.find("220") != std::string::npos);

    // client 1 sends USER
    sendCmd(fd1, "USER Anonymous");
    std::string r1 = readReply(fd1);
    cr_assert(r1.find("331") != std::string::npos);

    // client 2 sends USER
    sendCmd(fd2, "USER Anonymous");
    std::string r2 = readReply(fd2);
    cr_assert(r2.find("331") != std::string::npos);

    // both quit
    sendCmd(fd1, "QUIT");
    readReply(fd1);
    sendCmd(fd2, "QUIT");
    readReply(fd2);

    close(fd1);
    close(fd2);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stopServer(port);
    t.join();
}

// ========== QUIT disconnects client (mustLogOff path) ==========

Test(Server, quit_triggers_disconnect_via_must_log_off) {
    uint16_t port = 14206;
    MyFtp::SignalHandler::mustClose = false;

    std::thread t([port]() {
        try {
            MyFtp::Server server(port, "/tmp");
            server.start();
        }
        catch (...) {}
    });

    int fd = connectToServer(port);
    cr_assert_neq(fd, -1);
    readReply(fd); // 220

    sendCmd(fd, "QUIT");
    std::string r = readReply(fd);
    cr_assert(r.find("221") != std::string::npos);

    // After QUIT the server should close us, reading should return 0
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    char buf[64];
    ssize_t n = read(fd, buf, sizeof(buf));
    cr_assert_leq(n, 0);

    close(fd);
    stopServer(port);
    t.join();
}

// ========== SIGINT stops the server ==========

Test(Server, sigint_stops_server) {
    uint16_t port = 14207;
    MyFtp::SignalHandler::mustClose = false;

    std::thread t([port]() {
        try {
            MyFtp::Server server(port, "/tmp");
            server.start();
        }
        catch (...) {}
    });

    // make sure server is up
    int fd = connectToServer(port);
    cr_assert_neq(fd, -1);
    readReply(fd); // 220
    close(fd);

    // send SIGINT to stop
    MyFtp::SignalHandler::sigintHandler(SIGINT);

    // unblock poll with a connection
    int dummy = connectToServer(port, 5);
    if (dummy >= 0) close(dummy);

    t.join();
    MyFtp::SignalHandler::mustClose = false;
}

// ========== Commands before login ==========

Test(Server, commands_before_login) {
    uint16_t port = 14208;
    MyFtp::SignalHandler::mustClose = false;

    std::thread t([port]() {
        try {
            MyFtp::Server server(port, "/tmp");
            server.start();
        }
        catch (...) {}
    });

    int fd = connectToServer(port);
    cr_assert_neq(fd, -1);
    readReply(fd); // 220

    // CWD without login -> 530
    sendCmd(fd, "CWD /tmp");
    std::string r1 = readReply(fd);
    cr_assert(r1.find("530") != std::string::npos);

    // CDUP without login -> 530
    sendCmd(fd, "CDUP");
    std::string r2 = readReply(fd);
    cr_assert(r2.find("530") != std::string::npos);

    close(fd);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stopServer(port);
    t.join();
}

// ========== PASS without USER -> 332 ==========

Test(Server, pass_without_user) {
    uint16_t port = 14209;
    MyFtp::SignalHandler::mustClose = false;

    std::thread t([port]() {
        try {
            MyFtp::Server server(port, "/tmp");
            server.start();
        }
        catch (...) {}
    });

    int fd = connectToServer(port);
    cr_assert_neq(fd, -1);
    readReply(fd); // 220

    sendCmd(fd, "PASS");
    std::string r = readReply(fd);
    cr_assert(r.find("332") != std::string::npos);

    close(fd);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stopServer(port);
    t.join();
}

// ========== USER with no argument -> 501 ==========

Test(Server, user_no_arg) {
    uint16_t port = 14210;
    MyFtp::SignalHandler::mustClose = false;

    std::thread t([port]() {
        try {
            MyFtp::Server server(port, "/tmp");
            server.start();
        }
        catch (...) {}
    });

    int fd = connectToServer(port);
    cr_assert_neq(fd, -1);
    readReply(fd); // 220

    sendCmd(fd, "USER");
    std::string r = readReply(fd);
    cr_assert(r.find("501") != std::string::npos);

    close(fd);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stopServer(port);
    t.join();
}

