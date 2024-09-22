#pragma once

#include <fstream>
#include <string>
#include "utils.h"

class Stream : public std::fstream {
private:
    std::string path;
public:
    void setPath(std::string &&targetPath);
    NODISCARD std::string getPath() const;
};