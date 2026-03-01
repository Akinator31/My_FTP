#include <criterion/criterion.h>
#include <cstring>
#include "Errors/MyFtpErrors.h++"

Test(MyFtpErrors, incorrect_number_args) {
    MyFtp::MyFtpErrors err(MyFtp::IncorrectNumberArgs);
    cr_assert_str_eq(err.what(), "Incorrect number of arguments!");
}

Test(MyFtpErrors, incorrect_args) {
    MyFtp::MyFtpErrors err(MyFtp::IncorrectArgs);
    cr_assert_str_eq(err.what(), "Incorrect arguments!");
}

Test(MyFtpErrors, incorrect_port) {
    MyFtp::MyFtpErrors err(MyFtp::IncorrectPort);
    cr_assert_str_eq(err.what(), "Incorrect port!");
}

Test(MyFtpErrors, incorrect_path) {
    MyFtp::MyFtpErrors err(MyFtp::IncorrectPath);
    cr_assert_str_eq(err.what(), "Incorrect path!");
}

Test(MyFtpErrors, error_create_socket) {
    MyFtp::MyFtpErrors err(MyFtp::ErrorCreateSocket);
    cr_assert_str_eq(err.what(), "An error occurred while creating the server socket!");
}

Test(MyFtpErrors, error_bind_socket) {
    MyFtp::MyFtpErrors err(MyFtp::ErrorBindSocket);
    cr_assert_not_null(err.what());
}

Test(MyFtpErrors, error_poll_socket) {
    MyFtp::MyFtpErrors err(MyFtp::ErrorPollSocket);
    cr_assert_str_eq(err.what(), "An error occurred while polling the server socket!");
}

Test(MyFtpErrors, error_accept_socket) {
    MyFtp::MyFtpErrors err(MyFtp::ErrorAcceptSocket);
    cr_assert_str_eq(err.what(), "An error occurred while accepting connection from the server socket!");
}

Test(MyFtpErrors, error_read_socket) {
    MyFtp::MyFtpErrors err(MyFtp::ErrorReadSocket);
    cr_assert_str_eq(err.what(), "An error occurred while reading a socket!");
}

Test(MyFtpErrors, error_reply_code) {
    MyFtp::MyFtpErrors err(MyFtp::ErrorReplyCode);
    cr_assert_str_eq(err.what(), "This reply code doesn't exist!");
}

Test(MyFtpErrors, is_std_exception) {
    try {
        throw MyFtp::MyFtpErrors(MyFtp::IncorrectPort);
    }
    catch (const std::exception& e) {
        cr_assert_str_eq(e.what(), "Incorrect port!");
    }
}
