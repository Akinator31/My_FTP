#include <iostream>

#include "Errors/MyFtpErrors.h++"
#include "Server/Server.h++"
#include "Utils/Utils.h++"

int main(const int ac, char** av) {
    if (std::string(av[1]) == "--help")
        return MyFtp::Utils::printUsage();

    try {
        if (ac != 3)
            throw MyFtp::MyFtpErrors(MyFtp::IncorrectNumberArgs);

        MyFtp::Server server = MyFtp::Utils::loadServer(av);

        server.start();
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 84;
    }
}
