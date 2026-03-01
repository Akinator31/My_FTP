#include <criterion/criterion.h>
#include <sys/socket.h>
#include <unistd.h>
#include "Poller/Poller.h++"

Test(Poller, add_fd) {
    int fds[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    MyFtp::Poller poller;
    poller.add(fds[0], POLLIN);
    // adding same fd again should not crash
    poller.add(fds[0], POLLIN);
    close(fds[0]);
    close(fds[1]);
}

Test(Poller, remove_fd) {
    int fds[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    MyFtp::Poller poller;
    poller.add(fds[0], POLLIN);
    poller.remove(fds[0]);
    // removing again should not crash
    poller.remove(fds[0]);
    close(fds[0]);
    close(fds[1]);
}

Test(Poller, remove_nonexistent) {
    MyFtp::Poller poller;
    poller.remove(999);
}

Test(Poller, wait_timeout) {
    int fds[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    MyFtp::Poller poller;
    poller.add(fds[0], POLLIN);
    int result = poller.wait(1);
    cr_assert_eq(result, 0);
    close(fds[0]);
    close(fds[1]);
}

Test(Poller, is_readable) {
    int fds[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    MyFtp::Poller poller;
    poller.add(fds[0], POLLIN);

    const char* msg = "hi";
    write(fds[1], msg, 2);

    poller.wait(100);
    cr_assert(poller.isReadable(fds[0]));
    close(fds[0]);
    close(fds[1]);
}

Test(Poller, is_readable_no_data) {
    int fds[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    MyFtp::Poller poller;
    poller.add(fds[0], POLLIN);

    poller.wait(1);
    cr_assert_not(poller.isReadable(fds[0]));
    close(fds[0]);
    close(fds[1]);
}

Test(Poller, is_readable_unknown_fd) {
    MyFtp::Poller poller;
    cr_assert_not(poller.isReadable(9999));
}

Test(Poller, is_writable) {
    int fds[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    MyFtp::Poller poller;
    poller.add(fds[0], POLLOUT);

    poller.wait(100);
    cr_assert(poller.isWritable(fds[0]));
    close(fds[0]);
    close(fds[1]);
}

Test(Poller, is_writable_unknown_fd) {
    MyFtp::Poller poller;
    cr_assert_not(poller.isWritable(9999));
}

Test(Poller, is_invalid_closed_fd) {
    MyFtp::Poller poller;
    poller.add(9999, POLLIN);
    poller.wait(1);
    cr_assert(poller.isInvalid(9999));
}

Test(Poller, is_invalid_unknown_fd) {
    MyFtp::Poller poller;
    cr_assert_not(poller.isInvalid(9999));
}

Test(Poller, has_error_unknown_fd) {
    MyFtp::Poller poller;
    cr_assert_not(poller.hasError(9999));
}

Test(Poller, has_hangup) {
    int fds[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    MyFtp::Poller poller;
    poller.add(fds[0], POLLIN);
    close(fds[1]);
    poller.wait(100);
    cr_assert(poller.hasHangup(fds[0]));
    close(fds[0]);
}

Test(Poller, has_hangup_unknown_fd) {
    MyFtp::Poller poller;
    cr_assert_not(poller.hasHangup(9999));
}

Test(Poller, remove_swap_logic) {
    int fds1[2], fds2[2], fds3[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds1), 0);
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds2), 0);
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds3), 0);

    MyFtp::Poller poller;
    poller.add(fds1[0], POLLIN);
    poller.add(fds2[0], POLLIN);
    poller.add(fds3[0], POLLIN);

    // remove the first one, triggers swap with last
    poller.remove(fds1[0]);

    // the other two should still work
    write(fds2[1], "x", 1);
    poller.wait(100);
    cr_assert(poller.isReadable(fds2[0]));

    close(fds1[0]);
    close(fds1[1]);
    close(fds2[0]);
    close(fds2[1]);
    close(fds3[0]);
    close(fds3[1]);
}

Test(Poller, remove_last_element) {
    int fds[2];
    cr_assert_eq(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    MyFtp::Poller poller;
    poller.add(fds[0], POLLIN);
    poller.remove(fds[0]);
    // poller should be empty, wait should return 0 immediately
    int result = poller.wait(1);
    cr_assert_eq(result, 0);
    close(fds[0]);
    close(fds[1]);
}
