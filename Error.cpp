#include "Error.h"
#include <iostream>

void Error::fallWithError(const char *msg) {
    std::cerr << formError(msg);
    getchar();
    exit(1);
}

const char *Error::formError(const char *msg) {
    return (std::string{"ERROR::"} + msg).c_str();
}
