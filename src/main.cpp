#include <iostream>

#include "Errors/MyFtpErrors.h++"
#include "Server/Server.h++"
#include "Utils/Utils.h++"

int main(const int ac, char** av) {
    if (std::string(av[1]) == "--help")
        return my_ftp::Utils::printUsage();

    try {
        if (ac != 3)
            throw my_ftp::MyFtpErrors(my_ftp::IncorrectNumberArgs);

        my_ftp::Server server = my_ftp::Utils::loadServer(av);

        server.start();
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 84;
    }
}
