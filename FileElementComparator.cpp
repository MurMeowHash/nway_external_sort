#include "FileElementComparator.h"

bool FileElementComparator::operator()(const FileElement &lhs, const FileElement &rhs) const {
    return lhs.value > rhs.value;
}