#include "Random.h"
#include <algorithm>

Random::Random() : mt{rd()} {

}

std::vector<uint> Random::genRandomNumbers(ulong bytesCount, uint upperLimit) {
    std::uniform_int_distribution<uint> distribution{1, upperLimit};
    uint numCount{static_cast<uint>(bytesCount / sizeof(uint))};
    std::vector<uint> randomArray(numCount);
    for(int i = 0; i < numCount; i++) {
        randomArray[i] = distribution(mt);
    }
    return randomArray;
}