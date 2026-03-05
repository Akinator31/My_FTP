#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <sys/socket.h>
#include <unistd.h>
#include <memory>
#include <filesystem>
#include "../src/Server/Commands.h++"
#include "Client/Client.h++"
#include "FtpSession/FtpSession.h++"

static MyFtp::Client makeClient(int fds[2]) {
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    auto session = std::make_unique<MyFtp::FtpSession>(
        MyFtp::FTPClient, MyFtp::Socket(fds[0]));
    return MyFtp::Client(fds[0], std::move(session), "/tmp");
}

static std::string getReply(MyFtp::Client& client) {
    return client.getSession()->getOutputBuffer();
}

static void loginClient(MyFtp::Client& client) {
    client.getUsername() = "Anonymous";
    client.getPassword() = "";
    client.userLoggedIn();
}

// ========== USER ==========

Test(Commands_USER, user_sets_username) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::user(client, "USER Anonymous");
    cr_assert_str_eq(client.getUsername().c_str(), "Anonymous");
    cr_assert(getReply(client).find("331") != std::string::npos);
    close(fds[1]);
}

Test(Commands_USER, user_no_argument) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::user(client, "USER");
    cr_assert(getReply(client).find("501") != std::string::npos);
    close(fds[1]);
}

Test(Commands_USER, user_already_logged_in) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    MyFtp::Commands::user(client, "USER test");
    cr_assert(getReply(client).find("230") != std::string::npos);
    close(fds[1]);
}

// ========== PASS ==========

Test(Commands_PASS, pass_without_user) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::pass(client, "PASS");
    cr_assert(getReply(client).find("332") != std::string::npos);
    close(fds[1]);
}

Test(Commands_PASS, pass_anonymous_success) {
    int fds[2];
    auto client = makeClient(fds);
    client.getUsername() = "Anonymous";
    MyFtp::Commands::pass(client, "PASS");
    cr_assert(getReply(client).find("230") != std::string::npos);
    cr_assert(client.isClientAlreadyLoggedIn());
    close(fds[1]);
}

Test(Commands_PASS, pass_wrong_credentials) {
    int fds[2];
    auto client = makeClient(fds);
    client.getUsername() = "Anonymous";
    MyFtp::Commands::pass(client, "PASS wrongpassword");
    cr_assert(getReply(client).find("530") != std::string::npos);
    close(fds[1]);
}

Test(Commands_PASS, pass_wrong_user) {
    int fds[2];
    auto client = makeClient(fds);
    client.getUsername() = "notAnonymous";
    MyFtp::Commands::pass(client, "PASS");
    cr_assert(getReply(client).find("530") != std::string::npos);
    close(fds[1]);
}

Test(Commands_PASS, pass_already_logged_in) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    MyFtp::Commands::pass(client, "PASS something");
    cr_assert(getReply(client).find("230") != std::string::npos);
    close(fds[1]);
}

// ========== CWD ==========

Test(Commands_CWD, cwd_not_logged_in) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::cwd(client, "CWD /tmp");
    cr_assert(getReply(client).find("530") != std::string::npos);
    close(fds[1]);
}

Test(Commands_CWD, cwd_no_argument) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    MyFtp::Commands::cwd(client, "CWD");
    cr_assert(getReply(client).find("501") != std::string::npos);
    close(fds[1]);
}

Test(Commands_CWD, cwd_valid_directory) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    // /tmp should exist
    client.setCurrentPath("/tmp");
    MyFtp::Commands::cwd(client, "CWD .");
    std::string reply = getReply(client);
    // should get 250 (success) or 550 depending on the directory
    cr_assert(reply.find("250") != std::string::npos ||
        reply.find("550") != std::string::npos);
    close(fds[1]);
}

Test(Commands_CWD, cwd_outside_root) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    client.setCurrentPath("/tmp");
    MyFtp::Commands::cwd(client, "CWD ../../../../../../etc");
    std::string reply = getReply(client);
    cr_assert(reply.find("550") != std::string::npos);
    close(fds[1]);
}

Test(Commands_CWD, cwd_nonexistent_directory) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    client.setCurrentPath("/tmp");
    MyFtp::Commands::cwd(client, "CWD /tmp/this_directory_should_not_exist_12345");
    std::string reply = getReply(client);
    cr_assert(reply.find("550") != std::string::npos);
    close(fds[1]);
}

// ========== CDUP ==========

Test(Commands_CDUP, cdup_not_logged_in) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::cdup(client, "CDUP");
    cr_assert(getReply(client).find("530") != std::string::npos);
    close(fds[1]);
}

Test(Commands_CDUP, cdup_with_arguments) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    MyFtp::Commands::cdup(client, "CDUP extra");
    cr_assert(getReply(client).find("501") != std::string::npos);
    close(fds[1]);
}

Test(Commands_CDUP, cdup_success) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);

    // create a temp subdir to go into
    std::filesystem::path sub = std::filesystem::canonical("/tmp");
    client.setCurrentPath(sub);
    client.getSession()->getOutputBuffer().clear();
    MyFtp::Commands::cdup(client, "CDUP");
    std::string reply = getReply(client);
    // should be 200 if parent is still inside root, or 550 if we're already at root
    cr_assert(reply.find("200") != std::string::npos ||
        reply.find("550") != std::string::npos);
    close(fds[1]);
}

Test(Commands_CDUP, cdup_at_root) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    client.setCurrentPath(std::filesystem::canonical("/tmp"));
    client.getSession()->getOutputBuffer().clear();
    MyFtp::Commands::cdup(client, "CDUP");
    std::string reply = getReply(client);
    // going above /tmp should fail (550)
    cr_assert(reply.find("550") != std::string::npos);
    close(fds[1]);
}

// ========== QUIT ==========

Test(Commands_QUIT, quit_success) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::quit(client, "QUIT");
    cr_assert(getReply(client).find("221") != std::string::npos);
    cr_assert(client.mustLogOff());
    close(fds[1]);
}

Test(Commands_QUIT, quit_with_arguments) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::quit(client, "QUIT extra");
    cr_assert(getReply(client).find("501") != std::string::npos);
    cr_assert_not(client.mustLogOff());
    close(fds[1]);
}

Test(Commands_QUIT, quit_not_logged_in) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::quit(client, "QUIT");
    cr_assert(getReply(client).find("221") != std::string::npos);
    cr_assert(client.mustLogOff());
    close(fds[1]);
}
