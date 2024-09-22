#include "StreamException.h"

const std::string StreamException::STREAM_ERROR_MARKER{"STREAM::"};

StreamException::StreamException(const std::string &msg) {
    message = (STREAM_ERROR_MARKER + msg).c_str();
}

const char *StreamException::what() const noexcept {
    return message;
}
