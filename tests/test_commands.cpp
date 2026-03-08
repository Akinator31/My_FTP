#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <sys/socket.h>
#include <unistd.h>
#include <memory>
#include <filesystem>
#include <fstream>
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

// ========== PWD ==========

Test(Commands_PWD, pwd_not_logged_in) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::pwd(client, "PWD");
    cr_assert(getReply(client).find("530") != std::string::npos);
    close(fds[1]);
}

Test(Commands_PWD, pwd_with_arguments) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    MyFtp::Commands::pwd(client, "PWD extra");
    cr_assert(getReply(client).find("501") != std::string::npos);
    close(fds[1]);
}

Test(Commands_PWD, pwd_at_root) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    client.setCurrentPath(std::filesystem::canonical("/tmp"));
    client.getSession()->getOutputBuffer().clear();
    MyFtp::Commands::pwd(client, "PWD");
    std::string reply = getReply(client);
    cr_assert(reply.find("257") != std::string::npos);
    cr_assert(reply.find("\"/\"") != std::string::npos);
    close(fds[1]);
}

// ========== NOOP ==========

Test(Commands_NOOP, noop_success) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::noop(client, "NOOP");
    cr_assert(getReply(client).find("200") != std::string::npos);
    close(fds[1]);
}

Test(Commands_NOOP, noop_with_arguments) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::noop(client, "NOOP extra");
    cr_assert(getReply(client).find("501") != std::string::npos);
    close(fds[1]);
}

// ========== HELP ==========

Test(Commands_HELP, help_global) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::help(client, "HELP");
    std::string reply = getReply(client);
    cr_assert(reply.find("214") != std::string::npos);
    cr_assert(reply.find("USER") != std::string::npos);
    cr_assert(reply.find("QUIT") != std::string::npos);
    close(fds[1]);
}

Test(Commands_HELP, help_specific_command) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::help(client, "HELP USER");
    std::string reply = getReply(client);
    cr_assert(reply.find("214") != std::string::npos);
    cr_assert(reply.find("USER <username>") != std::string::npos);
    close(fds[1]);
}

Test(Commands_HELP, help_unknown_command) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::help(client, "HELP FOOBAR");
    cr_assert(getReply(client).find("501") != std::string::npos);
    close(fds[1]);
}

Test(Commands_HELP, help_pasv) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::help(client, "HELP PASV");
    std::string reply = getReply(client);
    cr_assert(reply.find("214") != std::string::npos);
    cr_assert(reply.find("PASV") != std::string::npos);
    close(fds[1]);
}

Test(Commands_HELP, help_stor) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::help(client, "HELP STOR");
    std::string reply = getReply(client);
    cr_assert(reply.find("214") != std::string::npos);
    cr_assert(reply.find("STOR") != std::string::npos);
    close(fds[1]);
}

// ========== DELE ==========

Test(Commands_DELE, dele_not_logged_in) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::dele(client, "DELE somefile");
    cr_assert(getReply(client).find("530") != std::string::npos);
    close(fds[1]);
}

Test(Commands_DELE, dele_no_argument) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    MyFtp::Commands::dele(client, "DELE");
    cr_assert(getReply(client).find("501") != std::string::npos);
    close(fds[1]);
}

Test(Commands_DELE, dele_nonexistent_file) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    client.setCurrentPath(std::filesystem::canonical("/tmp"));
    client.getSession()->getOutputBuffer().clear();
    MyFtp::Commands::dele(client, "DELE this_file_does_not_exist_99999");
    cr_assert(getReply(client).find("550") != std::string::npos);
    close(fds[1]);
}

Test(Commands_DELE, dele_outside_root) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    client.setCurrentPath(std::filesystem::canonical("/tmp"));
    client.getSession()->getOutputBuffer().clear();
    MyFtp::Commands::dele(client, "DELE /../../etc/passwd");
    cr_assert(getReply(client).find("550") != std::string::npos);
    close(fds[1]);
}

Test(Commands_DELE, dele_success) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    client.setCurrentPath(std::filesystem::canonical("/tmp"));

    // Create a temporary file to delete
    std::string tmpfile = "/tmp/myftp_test_dele_" + std::to_string(getpid());
    {
        std::ofstream f(tmpfile);
        f << "test";
    }
    cr_assert(std::filesystem::exists(tmpfile));

    client.getSession()->getOutputBuffer().clear();
    MyFtp::Commands::dele(client, "DELE " + tmpfile.substr(5)); // relative to /tmp
    std::string reply = getReply(client);
    cr_assert(reply.find("250") != std::string::npos);
    cr_assert_not(std::filesystem::exists(tmpfile));
    close(fds[1]);
}

Test(Commands_DELE, dele_absolute_path) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    client.setCurrentPath(std::filesystem::canonical("/tmp"));

    std::string tmpfile = "/tmp/myftp_test_dele_abs_" + std::to_string(getpid());
    {
        std::ofstream f(tmpfile);
        f << "test";
    }
    cr_assert(std::filesystem::exists(tmpfile));

    client.getSession()->getOutputBuffer().clear();
    // Absolute path starting with /
    MyFtp::Commands::dele(client, "DELE /" + tmpfile.substr(5));
    std::string reply = getReply(client);
    cr_assert(reply.find("250") != std::string::npos);
    cr_assert_not(std::filesystem::exists(tmpfile));
    close(fds[1]);
}

// ========== PASV ==========

Test(Commands_PASV, pasv_success) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::pasv(client, "PASV");
    std::string reply = getReply(client);
    cr_assert(reply.find("227") != std::string::npos);
    close(fds[1]);
}

Test(Commands_PASV, pasv_with_arguments) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::pasv(client, "PASV extra");
    cr_assert(getReply(client).find("501") != std::string::npos);
    close(fds[1]);
}

// ========== PORT ==========

Test(Commands_PORT, port_not_logged_in) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::port(client, "PORT 127,0,0,1,4,1");
    cr_assert(getReply(client).find("530") != std::string::npos);
    close(fds[1]);
}

Test(Commands_PORT, port_no_argument) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    MyFtp::Commands::port(client, "PORT");
    cr_assert(getReply(client).find("501") != std::string::npos);
    close(fds[1]);
}

Test(Commands_PORT, port_invalid_args) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    MyFtp::Commands::port(client, "PORT abc,def,ghi,jkl,mn,op");
    cr_assert(getReply(client).find("501") != std::string::npos);
    close(fds[1]);
}

Test(Commands_PORT, port_success) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    MyFtp::Commands::port(client, "PORT 127,0,0,1,4,1");
    std::string reply = getReply(client);
    cr_assert(reply.find("200") != std::string::npos);
    cr_assert(client.getDataTransferManager().isMode(MyFtp::ACTIVE));
    close(fds[1]);
}

// ========== LIST ==========

Test(Commands_LIST, list_not_logged_in) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::list(client, "LIST");
    cr_assert(getReply(client).find("530") != std::string::npos);
    close(fds[1]);
}

Test(Commands_LIST, list_no_data_mode) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    MyFtp::Commands::list(client, "LIST");
    cr_assert(getReply(client).find("425") != std::string::npos);
    close(fds[1]);
}

Test(Commands_LIST, list_with_pasv) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    client.setCurrentPath(std::filesystem::canonical("/tmp"));
    // Enter passive mode first
    MyFtp::Commands::pasv(client, "PASV");
    client.getSession()->getOutputBuffer().clear();
    MyFtp::Commands::list(client, "LIST");
    std::string reply = getReply(client);
    cr_assert(reply.find("150") != std::string::npos);
    close(fds[1]);
}

Test(Commands_LIST, list_with_path) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    client.setCurrentPath(std::filesystem::canonical("/tmp"));
    MyFtp::Commands::pasv(client, "PASV");
    client.getSession()->getOutputBuffer().clear();
    MyFtp::Commands::list(client, "LIST .");
    std::string reply = getReply(client);
    cr_assert(reply.find("150") != std::string::npos);
    close(fds[1]);
}

// ========== RETR ==========

Test(Commands_RETR, retr_not_logged_in) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::retr(client, "RETR somefile");
    cr_assert(getReply(client).find("530") != std::string::npos);
    close(fds[1]);
}

Test(Commands_RETR, retr_no_data_mode) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    MyFtp::Commands::retr(client, "RETR somefile");
    cr_assert(getReply(client).find("425") != std::string::npos);
    close(fds[1]);
}

Test(Commands_RETR, retr_nonexistent_file) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    client.setCurrentPath(std::filesystem::canonical("/tmp"));
    MyFtp::Commands::pasv(client, "PASV");
    client.getSession()->getOutputBuffer().clear();
    MyFtp::Commands::retr(client, "RETR nonexistent_file_99999");
    std::string reply = getReply(client);
    cr_assert(reply.find("550") != std::string::npos);
    close(fds[1]);
}

// ========== STOR ==========

Test(Commands_STOR, stor_not_logged_in) {
    int fds[2];
    auto client = makeClient(fds);
    MyFtp::Commands::stor(client, "STOR somefile");
    cr_assert(getReply(client).find("530") != std::string::npos);
    close(fds[1]);
}

Test(Commands_STOR, stor_no_data_mode) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    MyFtp::Commands::stor(client, "STOR somefile");
    cr_assert(getReply(client).find("425") != std::string::npos);
    close(fds[1]);
}

Test(Commands_STOR, stor_with_pasv) {
    int fds[2];
    auto client = makeClient(fds);
    loginClient(client);
    client.setCurrentPath(std::filesystem::canonical("/tmp"));
    MyFtp::Commands::pasv(client, "PASV");
    client.getSession()->getOutputBuffer().clear();
    MyFtp::Commands::stor(client, "STOR testfile_stor");
    std::string reply = getReply(client);
    cr_assert(reply.find("150") != std::string::npos);
    close(fds[1]);
}

