#include <criterion/criterion.h>
#include <criterion/redirect.h>
#include <filesystem>
#include "Utils/Utils.h++"
#include "Errors/MyFtpErrors.h++"

// ========== printUsage ==========

static void redirect_stdout(void) {
    cr_redirect_stdout();
}

Test(Utils_printUsage, returns_zero, .init = redirect_stdout) {
    int ret = MyFtp::Utils::printUsage();
    cr_assert_eq(ret, 0);
}

Test(Utils_printUsage, prints_usage, .init = redirect_stdout) {
    MyFtp::Utils::printUsage();
    fflush(stdout);
    cr_assert_stdout_eq_str(
        "USAGE: ./myftp port path\n"
        " port is the port number on which the server socket listens\n"
        " path is the path to the home directory for the Anonymous user\n"
    );
}

// ========== loadServer ==========

Test(Utils_loadServer, valid_args) {
    char arg0[] = "myftp";
    char arg1[] = "0";
    char arg2[] = "/tmp";
    char* av[] = {arg0, arg1, arg2, nullptr};
    try {
        MyFtp::Server server = MyFtp::Utils::loadServer(av);
        (void)server;
    }
    catch (...) {
        cr_assert_fail("Should not throw with valid args");
    }
}

Test(Utils_loadServer, invalid_port) {
    char arg0[] = "myftp";
    char arg1[] = "notaport";
    char arg2[] = "/tmp";
    char* av[] = {arg0, arg1, arg2, nullptr};
    try {
        MyFtp::Utils::loadServer(av);
        cr_assert_fail("Should have thrown");
    }
    catch (const MyFtp::MyFtpErrors& e) {
        cr_assert_str_eq(e.what(), "Incorrect port!");
    }
}

Test(Utils_loadServer, invalid_path) {
    char arg0[] = "myftp";
    char arg1[] = "4242";
    char arg2[] = "/this/path/does/not/exist/at/all";
    char* av[] = {arg0, arg1, arg2, nullptr};
    try {
        MyFtp::Utils::loadServer(av);
        cr_assert_fail("Should have thrown");
    }
    catch (const MyFtp::MyFtpErrors& e) {
        cr_assert_str_eq(e.what(), "Incorrect path!");
    }
}

// ========== isPathInsideTheRootPath ==========

Test(Utils_isPath, inside_root) {
    auto root = std::filesystem::canonical("/tmp");
    auto path = std::filesystem::canonical("/tmp");
    cr_assert(MyFtp::Utils::isPathInsideTheRootPath(root, path));
}

Test(Utils_isPath, subdir_inside_root) {
    auto root = std::filesystem::canonical("/tmp");
    // /tmp itself is inside /tmp
    cr_assert(MyFtp::Utils::isPathInsideTheRootPath(root, root));
}

Test(Utils_isPath, outside_root) {
    auto root = std::filesystem::canonical("/tmp");
    auto path = std::filesystem::canonical("/");
    cr_assert_not(MyFtp::Utils::isPathInsideTheRootPath(root, path));
}

Test(Utils_isPath, etc_outside_tmp) {
    auto root = std::filesystem::canonical("/tmp");
    auto path = std::filesystem::canonical("/etc");
    cr_assert_not(MyFtp::Utils::isPathInsideTheRootPath(root, path));
}

Test(Utils_isPath, same_path) {
    auto root = std::filesystem::canonical("/tmp");
    cr_assert(MyFtp::Utils::isPathInsideTheRootPath(root, root));
}

// ========== parsePORTCommand ==========

Test(Utils_parsePORT, valid_command) {
    auto result = MyFtp::Utils::parsePORTCommand("PORT 127,0,0,1,4,1");
    cr_assert(result.has_value());
    auto arr = result.value();
    cr_assert_eq(arr[0], 127);
    cr_assert_eq(arr[1], 0);
    cr_assert_eq(arr[2], 0);
    cr_assert_eq(arr[3], 1);
    cr_assert_eq(arr[4], 4);
    cr_assert_eq(arr[5], 1);
}

Test(Utils_parsePORT, high_port) {
    auto result = MyFtp::Utils::parsePORTCommand("PORT 192,168,1,100,200,100");
    cr_assert(result.has_value());
    auto arr = result.value();
    cr_assert_eq(arr[0], 192);
    cr_assert_eq(arr[1], 168);
    cr_assert_eq(arr[2], 1);
    cr_assert_eq(arr[3], 100);
    // port = 200*256 + 100 = 51364
    cr_assert_eq(arr[4], 200);
    cr_assert_eq(arr[5], 100);
}

Test(Utils_parsePORT, invalid_command_letters) {
    auto result = MyFtp::Utils::parsePORTCommand("PORT abc,def,ghi,jkl,mn,op");
    cr_assert_not(result.has_value());
}

Test(Utils_parsePORT, without_prefix) {
    // The function also handles strings without the "PORT " prefix
    auto result = MyFtp::Utils::parsePORTCommand("127,0,0,1,4,1");
    cr_assert(result.has_value());
    auto arr = result.value();
    cr_assert_eq(arr[0], 127);
    cr_assert_eq(arr[5], 1);
}

Test(Utils_parsePORT, zeros) {
    auto result = MyFtp::Utils::parsePORTCommand("PORT 0,0,0,0,0,0");
    cr_assert(result.has_value());
    auto arr = result.value();
    for (int i = 0; i < 6; i++)
        cr_assert_eq(arr[i], 0);
}

// ========== getOutputCommand ==========

Test(Utils_getOutputCommand, echo_command) {
    std::string result = MyFtp::Utils::getOutputCommand("/bin/echo hello");
    cr_assert(result.find("hello") != std::string::npos);
    // Should end with \r\n (CRLF)
    cr_assert(result.find("\r\n") != std::string::npos);
}

Test(Utils_getOutputCommand, ls_command) {
    std::string result = MyFtp::Utils::getOutputCommand("/bin/ls /tmp");
    // Should return something (non-empty)
    cr_assert_not(result.empty());
}

Test(Utils_getOutputCommand, multiline_output) {
    std::string result = MyFtp::Utils::getOutputCommand("/bin/echo -e 'line1\\nline2'");
    cr_assert(result.find("line1\r\n") != std::string::npos);
    cr_assert(result.find("line2\r\n") != std::string::npos);
}

