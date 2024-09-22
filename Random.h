#pragma once

#include <vector>
#include <random>
#include "utils.h"

class Random {
private:
    std::random_device rd;
    std::mt19937 mt;
public:
    Random();
    std::vector<uint> genRandomNumbers(ulong bytesCount, uint upperLimit);
};