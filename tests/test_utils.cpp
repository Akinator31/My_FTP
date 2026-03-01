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
