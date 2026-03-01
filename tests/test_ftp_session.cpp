#include <criterion/criterion.h>
#include <sys/socket.h>
#include <unistd.h>
#include "FtpSession/FtpSession.h++"
#include "Errors/MyFtpErrors.h++"

Test(FtpSession, create_server_session) {
    int fds[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    MyFtp::FtpSession session(MyFtp::FTPServer, MyFtp::Socket(fds[0]));
    cr_assert_eq(session.getControlSocket().fd(), fds[0]);
    close(fds[1]);
}

Test(FtpSession, create_client_session) {
    int fds[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    MyFtp::FtpSession session(MyFtp::FTPClient, MyFtp::Socket(fds[0]));
    cr_assert_eq(session.getControlSocket().fd(), fds[0]);
    close(fds[1]);
}

Test(FtpSession, invalid_socket_throws) {
    try {
        MyFtp::FtpSession session(MyFtp::FTPServer, MyFtp::Socket(-1));
        cr_assert_fail("Should have thrown");
    }
    catch (const MyFtp::MyFtpErrors& e) {
        cr_assert_not_null(e.what());
    }
}

Test(FtpSession, command_buffer_empty_initially) {
    int fds[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    MyFtp::FtpSession session(MyFtp::FTPServer, MyFtp::Socket(fds[0]));
    cr_assert(session.getCommandBuffer().empty());
    close(fds[1]);
}

Test(FtpSession, output_buffer_empty_initially) {
    int fds[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    MyFtp::FtpSession session(MyFtp::FTPServer, MyFtp::Socket(fds[0]));
    cr_assert(session.getOutputBuffer().empty());
    close(fds[1]);
}

Test(FtpSession, command_buffer_append) {
    int fds[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    MyFtp::FtpSession session(MyFtp::FTPServer, MyFtp::Socket(fds[0]));
    session.getCommandBuffer().append("USER Anonymous\r\n");
    cr_assert_str_eq(session.getCommandBuffer().c_str(), "USER Anonymous\r\n");
    close(fds[1]);
}

Test(FtpSession, output_buffer_append) {
    int fds[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    MyFtp::FtpSession session(MyFtp::FTPServer, MyFtp::Socket(fds[0]));
    session.getOutputBuffer().append("220 Ready\r\n");
    cr_assert_str_eq(session.getOutputBuffer().c_str(), "220 Ready\r\n");
    close(fds[1]);
}
