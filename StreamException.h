#pragma once

#include <exception>
#include <string>
#include "utils.h"

class StreamException : public std::exception {
private:
    static const std::string STREAM_ERROR_MARKER;
    const char *message;
public:
    explicit StreamException(const std::string &msg);
    NODISCARD const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override;
};