//
// Created by pavel on 19/02/2026.
//

#pragma once
#include "Server/Server.h++"

namespace my_ftp {
    class Utils {
    public:
        static Server loadServer(char** av);
        static int printUsage();
    };
}
