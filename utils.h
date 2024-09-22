#pragma once

#define NODISCARD [[nodiscard]]

#include <fstream>

typedef unsigned int uint;
typedef unsigned long long ulong;

static constexpr ulong MB_TO_B_CONVERSION_FACTOR{1048576};
static constexpr ulong GB_TO_B_CONVERSION_FACTOR{1073741824};

ulong mb_to_b(ulong mb);
ulong gb_to_b(ulong gb);

template<typename I, typename T>
void advanceIterator(I &targetIterator, T &targetArray) {
    if(++targetIterator == targetArray.end()) {
        targetIterator = targetArray.begin();
    }
}

void printFile(const char *path);
bool checkSorted(const char *targetFile);
ulong getFileSize(std::ifstream &target);