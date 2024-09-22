#pragma once

class Error {
public:
    static void fallWithError(const char *msg);
    static const char *formError(const char *msg);
};