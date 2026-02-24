//
// Created by pavel on 19/02/2026.
//

#include "MyFtpErrors.h++"

namespace MyFtp {
    MyFtpErrors::MyFtpErrors(const MyFtpErrorType type) {
        this->_errorMessage = this->_errorMap[type];
    }

    const char* MyFtpErrors::what() const noexcept {
        return this->_errorMessage.c_str();
    }
}
