#include "Stream.h"

void Stream::setPath(std::string &&targetPath) {
    path = targetPath;
}

std::string Stream::getPath() const {
    return path;
}
