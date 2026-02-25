//
// Created by pavel on 19/02/2026.
//

#pragma once
#include "Server/Server.h++"

namespace MyFtp {
    class Utils {
    public:
        static Server loadServer(char** av);
        static int printUsage();
        static bool isPathInsideTheRootPath(const std::filesystem::path& rootPath, const std::filesystem::path& path);
    };
}
