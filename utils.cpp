#include "utils.h"
#include <fstream>
#include <sstream>
#include <iostream>

ulong mb_to_b(ulong mb) {
    return mb * MB_TO_B_CONVERSION_FACTOR;
}

ulong gb_to_b(ulong gb) {
    return gb * GB_TO_B_CONVERSION_FACTOR;
}

void printFile(const char *path) {
    std::ifstream is{path, std::ios::binary};
    if(!is) {
        return;
    }
    uint num, count{0};
    while(is.read(reinterpret_cast<char *>(&num), sizeof(uint))) {
        std::cout << num << ' ';
        count++;
    }
    std::cout<<"\n\nCOUNT:\n\n" << count << "\n" << is.eof();
    is.close();
}

bool checkSorted(const char *targetFile) {
    std::ifstream is{targetFile, std::ios::binary};
    uint prevNum{0}, curNum;
    bool isSorted{true};
    while(is.read(reinterpret_cast<char *>(&curNum), sizeof(curNum))) {
        if(curNum < prevNum) {
            isSorted = false;
            break;
        }
        prevNum = curNum;
    }
    is.close();
    return isSorted;
}

ulong getFileSize(std::ifstream &target) {
    ulong fileSize;
    auto currentPos = target.tellg();
    target.seekg(0, std::ios::end);
    fileSize = target.tellg();
    target.seekg(currentPos);
    return fileSize;
}