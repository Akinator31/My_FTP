#include <criterion/criterion.h>
#include <sys/socket.h>
#include <unistd.h>
#include <memory>
#include <filesystem>
#include "Client/Client.h++"
#include "FtpSession/FtpSession.h++"
#include "Errors/MyFtpErrors.h++"

static MyFtp::Client makeClient(int fds[2]) {
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    auto session = std::make_unique<MyFtp::FtpSession>(
        MyFtp::FTPClient, MyFtp::Socket(fds[0]));
    return MyFtp::Client(fds[0], std::move(session), "/tmp");
}

// --- Constructor ---

Test(Client, constructor_sets_root_path) {
    int fds[2];
    auto client = makeClient(fds);
    cr_assert_str_eq(client.getRootPath().c_str(),
                     std::filesystem::canonical("/tmp").c_str());
    close(fds[1]);
}

Test(Client, constructor_sets_current_path) {
    int fds[2];
    auto client = makeClient(fds);
    cr_assert_str_eq(client.getCurrentPath().c_str(), "/tmp");
    close(fds[1]);
}

// --- Getters / Setters ---

Test(Client, get_session_not_null) {
    int fds[2];
    auto client = makeClient(fds);
    cr_assert_not_null(client.getSession().get());
    close(fds[1]);
}

Test(Client, username_default_empty) {
    int fds[2];
    auto client = makeClient(fds);
    cr_assert(client.getUsername().empty());
    close(fds[1]);
}

Test(Client, set_username) {
    int fds[2];
    auto client = makeClient(fds);
    client.getUsername() = "Anonymous";
    cr_assert_str_eq(client.getUsername().c_str(), "Anonymous");
    close(fds[1]);
}

Test(Client, get_password_default) {
    int fds[2];
    auto client = makeClient(fds);
    cr_assert_str_eq(client.getPassword().c_str(), "placeHolder");
    close(fds[1]);
}

Test(Client, set_password) {
    int fds[2];
    auto client = makeClient(fds);
    client.getPassword() = "secret";
    cr_assert_str_eq(client.getPassword().c_str(), "secret");
    close(fds[1]);
}

Test(Client, set_current_path) {
    int fds[2];
    auto client = makeClient(fds);
    client.setCurrentPath("/tmp/subdir");
    cr_assert_str_eq(client.getCurrentPath().c_str(), "/tmp/subdir");
    close(fds[1]);
}

// --- Login state ---

Test(Client, not_logged_in_by_default) {
    int fds[2];
    auto client = makeClient(fds);
    cr_assert_not(client.isClientAlreadyLoggedIn());
    close(fds[1]);
}

Test(Client, user_logged_in) {
    int fds[2];
    auto client = makeClient(fds);
    client.userLoggedIn();
    cr_assert(client.isClientAlreadyLoggedIn());
    close(fds[1]);
}

// --- Disconnect ---

Test(Client, must_log_off_false_by_default) {
    int fds[2];
    auto client = makeClient(fds);
    cr_assert_not(client.mustLogOff());
    close(fds[1]);
}

Test(Client, disconnect_sets_must_log_off) {
    int fds[2];
    auto client = makeClient(fds);
    client.disconnect();
    cr_assert(client.mustLogOff());
    close(fds[1]);
}

// --- sendReply ---

Test(Client, send_reply_ok) {
    int fds[2];
    auto client = makeClient(fds);
    client.sendReply(MyFtp::COMMAND_OK_200);
    std::string& out = client.getSession()->getOutputBuffer();
    cr_assert_str_eq(out.c_str(), "200 Command okay.\r\n");
    close(fds[1]);
}

Test(Client, send_reply_220) {
    int fds[2];
    auto client = makeClient(fds);
    client.sendReply(MyFtp::SERVICE_READY_220);
    std::string& out = client.getSession()->getOutputBuffer();
    cr_assert_str_eq(out.c_str(), "220 Service ready for new user.\r\n");
    close(fds[1]);
}

Test(Client, send_reply_221) {
    int fds[2];
    auto client = makeClient(fds);
    client.sendReply(MyFtp::SERVICE_CLOSING_221);
    std::string& out = client.getSession()->getOutputBuffer();
    cr_assert_str_eq(out.c_str(), "221 Service closing control connection.\r\n");
    close(fds[1]);
}

Test(Client, send_reply_230) {
    int fds[2];
    auto client = makeClient(fds);
    client.sendReply(MyFtp::USER_LOGGED_IN_230);
    std::string& out = client.getSession()->getOutputBuffer();
    cr_assert_str_eq(out.c_str(), "230 User logged in, proceed.\r\n");
    close(fds[1]);
}

Test(Client, send_reply_250) {
    int fds[2];
    auto client = makeClient(fds);
    client.sendReply(MyFtp::REQUEST_FILE_ACTION_OK_250);
    std::string& out = client.getSession()->getOutputBuffer();
    cr_assert_str_eq(out.c_str(), "250 Requested file action okay, completed.\r\n");
    close(fds[1]);
}

Test(Client, send_reply_331) {
    int fds[2];
    auto client = makeClient(fds);
    client.sendReply(MyFtp::USERNAME_OK_331);
    std::string& out = client.getSession()->getOutputBuffer();
    cr_assert_str_eq(out.c_str(), "331 User name okay, need password.\r\n");
    close(fds[1]);
}

Test(Client, send_reply_332) {
    int fds[2];
    auto client = makeClient(fds);
    client.sendReply(MyFtp::NEED_ACCOUNT_332);
    std::string& out = client.getSession()->getOutputBuffer();
    cr_assert_str_eq(out.c_str(), "332 Need account for login.\r\n");
    close(fds[1]);
}

Test(Client, send_reply_500) {
    int fds[2];
    auto client = makeClient(fds);
    client.sendReply(MyFtp::SYNTAX_ERROR_COMMAND_500);
    std::string& out = client.getSession()->getOutputBuffer();
    cr_assert_str_eq(out.c_str(), "500 Syntax error, command unrecognized.\r\n");
    close(fds[1]);
}

Test(Client, send_reply_501) {
    int fds[2];
    auto client = makeClient(fds);
    client.sendReply(MyFtp::SYNTAX_ERROR_ARGS_501);
    std::string& out = client.getSession()->getOutputBuffer();
    cr_assert_str_eq(out.c_str(), "501 Syntax error in parameters or arguments.\r\n");
    close(fds[1]);
}

Test(Client, send_reply_530) {
    int fds[2];
    auto client = makeClient(fds);
    client.sendReply(MyFtp::NOT_LOGGED_IN_530);
    std::string& out = client.getSession()->getOutputBuffer();
    cr_assert_str_eq(out.c_str(), "530 Not logged in.\r\n");
    close(fds[1]);
}

Test(Client, send_reply_550) {
    int fds[2];
    auto client = makeClient(fds);
    client.sendReply(MyFtp::FILE_UNAVAILABLE_550);
    std::string& out = client.getSession()->getOutputBuffer();
    cr_assert_str_eq(out.c_str(), "550 Requested action not taken.\r\n");
    close(fds[1]);
}

Test(Client, send_reply_multiple) {
    int fds[2];
    auto client = makeClient(fds);
    client.sendReply(MyFtp::COMMAND_OK_200);
    client.sendReply(MyFtp::NOT_LOGGED_IN_530);
    std::string& out = client.getSession()->getOutputBuffer();
    cr_assert_str_eq(out.c_str(), "200 Command okay.\r\n530 Not logged in.\r\n");
    close(fds[1]);
}

// --- readIncoming ---

Test(Client, read_incoming_ok) {
    int fds[2];
    auto client = makeClient(fds);
    const char* msg = "USER test\r\n";
    write(fds[1], msg, strlen(msg));
    auto result = client.readIncoming();
    cr_assert_eq(result, MyFtp::Client::Ok);
    cr_assert_str_eq(client.getSession()->getCommandBuffer().c_str(), "USER test\r\n");
    close(fds[1]);
}

Test(Client, read_incoming_disconnected) {
    int fds[2];
    auto client = makeClient(fds);
    close(fds[1]);
    auto result = client.readIncoming();
    cr_assert_eq(result, MyFtp::Client::Disconnected);
}

Test(Client, read_incoming_error) {
    int fds[2];
    auto client = makeClient(fds);
    // close our side of the socket to force an error on read
    client.getSession()->getControlSocket().close();
    close(fds[1]);
    auto result = client.readIncoming();
    cr_assert_eq(result, MyFtp::Client::Error);
}

// --- nextCommand ---

Test(Client, next_command_none) {
    int fds[2];
    auto client = makeClient(fds);
    auto cmd = client.nextCommand();
    cr_assert_not(cmd.has_value());
    close(fds[1]);
}

Test(Client, next_command_incomplete) {
    int fds[2];
    auto client = makeClient(fds);
    client.getSession()->getCommandBuffer() = "USER test";
    auto cmd = client.nextCommand();
    cr_assert_not(cmd.has_value());
    close(fds[1]);
}

Test(Client, next_command_single) {
    int fds[2];
    auto client = makeClient(fds);
    client.getSession()->getCommandBuffer() = "USER test\r\n";
    auto cmd = client.nextCommand();
    cr_assert(cmd.has_value());
    cr_assert_str_eq(cmd->c_str(), "USER test");
    cr_assert(client.getSession()->getCommandBuffer().empty());
    close(fds[1]);
}

Test(Client, next_command_multiple) {
    int fds[2];
    auto client = makeClient(fds);
    client.getSession()->getCommandBuffer() = "USER test\r\nPASS\r\n";
    auto cmd1 = client.nextCommand();
    cr_assert(cmd1.has_value());
    cr_assert_str_eq(cmd1->c_str(), "USER test");
    auto cmd2 = client.nextCommand();
    cr_assert(cmd2.has_value());
    cr_assert_str_eq(cmd2->c_str(), "PASS");
    auto cmd3 = client.nextCommand();
    cr_assert_not(cmd3.has_value());
    close(fds[1]);
}

// --- flushOutput ---

Test(Client, flush_output_sends_data) {
    int fds[2];
    auto client = makeClient(fds);
    client.sendReply(MyFtp::COMMAND_OK_200);
    client.flushOutput();

    char buf[256] = {};
    ssize_t rd = read(fds[1], buf, sizeof(buf) - 1);
    cr_assert_gt(rd, 0);
    cr_assert_str_eq(buf, "200 Command okay.\r\n");
    cr_assert(client.getSession()->getOutputBuffer().empty());
    close(fds[1]);
}

Test(Client, flush_output_empty) {
    int fds[2];
    auto client = makeClient(fds);
    client.flushOutput();
    cr_assert(client.getSession()->getOutputBuffer().empty());
    close(fds[1]);
}
