#pragma once
#include "FileElement.h"

struct FileElementComparator {
    bool operator()(const FileElement &lhs, const FileElement &rhs) const;
};