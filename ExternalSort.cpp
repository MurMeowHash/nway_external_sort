#include "ExternalSort.h"
#include "StreamException.h"
#include "Error.h"
#include <sstream>
#include <algorithm>
#include <queue>
#include <cmath>
#include <filesystem>
#include "FileElement.h"
#include "FileElementComparator.h"

const std::string ExternalSort::DIRECTORY{"../files/"};
const std::string ExternalSort::EXTENSION{".dat"};

ExternalSort::ExternalSort() : errorLog{nullptr}, errorState{false} {
    activeHeap = new std::priority_queue<FileElement, std::vector<FileElement>, FileElementComparator>;
    sideHeap = new std::priority_queue<FileElement, std::vector<FileElement>, FileElementComparator>;
    currentFiles = new std::array<Stream, HELP_FILES_COUNT>;
    targetFiles = new std::array<Stream, HELP_FILES_COUNT>;
}

void ExternalSort::nWayMergeSort(const char *path, const char *dest) {
    try {
        createHelpFiles(path);
        std::array<ulong, HELP_FILES_COUNT> currentFilesLength{0};
        auto currentFilesLengthIterator = currentFilesLength.begin();
        auto currentStreamIterator = currentFiles->begin();
        uint prevNum{0}, currentNum;
        bool eofReached{false};
        while(!eofReached) {
            if(!A.read(reinterpret_cast<char *>(&currentNum), sizeof(currentNum))) {
                eofReached = true;
            } else {
                if(currentNum < prevNum) {
                    advanceIterator(currentStreamIterator, *currentFiles);
                    advanceIterator(currentFilesLengthIterator, currentFilesLength);
                }
                currentStreamIterator->write(reinterpret_cast<char *>(&currentNum), sizeof(currentNum));
                (*currentFilesLengthIterator)++;
                prevNum = currentNum;
            }
        }
        A.close();
        uint sortedFileIndex = merge(currentFilesLength);
        terminateFiles();
        writeDestFile(sortedFileIndex, dest);
    } catch(StreamException &ex) {
        errorState = true;
        errorLog = Error::formError(ex.what());
    }
}

void ExternalSort::modifiedNWayMergeSort(const char *path, const char *dest) {
    try {
        createHelpFiles(path);
        std::array<ulong, HELP_FILES_COUNT> currentFilesLength{0};
        auto currentFilesLengthIterator = currentFilesLength.begin();
        auto currentStreamIterator = currentFiles->begin();
        ulong divisionBytes = getFileDivisionBytes(COUNT_FILE_PARTS);
        auto currentBlock = new std::vector<uint>(static_cast<uint>(divisionBytes / sizeof(uint)));
        long long readBytes;
        while(!A.eof()) {
            A.read(reinterpret_cast<char *>(currentBlock->data()), static_cast<long long>(divisionBytes));
            readBytes = A.gcount();
            if(divisionBytes != readBytes) {
                currentBlock->resize(readBytes / sizeof(uint));
            }
            std::sort(currentBlock->begin(), currentBlock->end());
            currentStreamIterator->write(reinterpret_cast<char *>(currentBlock->data()), readBytes);
            (*currentFilesLengthIterator) += currentBlock->size();
            advanceIterator(currentStreamIterator, *currentFiles);
            advanceIterator(currentFilesLengthIterator, currentFilesLength);
        }
        delete currentBlock;
        A.close();
        uint sortedFileIndex = merge(currentFilesLength);
        terminateFiles();
        writeDestFile(sortedFileIndex, dest);
    } catch(StreamException &ex) {
        errorState = true;
        errorLog = Error::formError(ex.what());
    }
}

bool ExternalSort::checkOneSeriesLeft(const std::array<ulong, HELP_FILES_COUNT> &currentFilesLength, uint &destFile) {
    uint countEmptyFiles{0};
    for(int i = 0; i < HELP_FILES_COUNT; i++) {
        if(currentFilesLength[i] == 0) {
            countEmptyFiles++;
        } else {
            destFile = i;
        }
    }
    return countEmptyFiles == (HELP_FILES_COUNT - 1);
}

void ExternalSort::readNext(std::array<ulong, HELP_FILES_COUNT> &currentFilesIndices,
                            std::array<ulong, HELP_FILES_COUNT> currentFilesLength,
                            const FileElement &prevElement,
                            std::array<bool, HELP_FILES_COUNT> &seriesEnded) {
    uint currentNum;
    if(currentFilesIndices[prevElement.index] < currentFilesLength[prevElement.index]
    && !seriesEnded[prevElement.index]) {
        (*currentFiles)[prevElement.index].read(reinterpret_cast<char *>(&currentNum), sizeof(currentNum));
        currentFilesIndices[prevElement.index]++;
        if(currentNum < prevElement.value) {
            seriesEnded[prevElement.index] = true;
            sideHeap->emplace(currentNum, prevElement.index);
        } else {
            activeHeap->emplace(currentNum, prevElement.index);
        }
    }
}

ExternalSort::~ExternalSort() {
    delete activeHeap;
    delete sideHeap;
    delete currentFiles;
    delete targetFiles;
}

bool ExternalSort::fail() const {
    return errorState;
}

const char *ExternalSort::getErrorLog() const {
    return errorLog;
}

void ExternalSort::createHelpFiles(const char *path) {
    A.open(path, std::ios::binary);
    if(!A) {
        throw StreamException{"FILE_A::FAILED_TO_OPEN"};
    }
    std::ios::openmode mode = std::ios::in | std::ios::out | std::ios::trunc | std::ios::binary;
    const char *currentFilesName{"B"};
    const char *targetFilesName{"C"};
    std::stringstream resultingPath;
    for(uint i = 0; i < HELP_FILES_COUNT; i++) {
        resultingPath.str(std::string());
        resultingPath << DIRECTORY << currentFilesName << i + 1 << EXTENSION;
        (*currentFiles)[i].open(resultingPath.str(), mode);
        (*currentFiles)[i].setPath(std::string(resultingPath.str()));
        resultingPath.str(std::string());
        resultingPath << DIRECTORY << targetFilesName << i + 1 << EXTENSION;
        (*targetFiles)[i].open(resultingPath.str(), mode);
        (*targetFiles)[i].setPath(std::string(resultingPath.str()));
        if(!(*currentFiles)[i]) {
            throw StreamException{"B" + std::to_string(i) + "::FAILED_TO_OPEN"};
        }
        if(!(*targetFiles)[i]) {
            throw StreamException{"C" + std::to_string(i) + "::FAILED_TO_OPEN"};
        }
    }
}

uint ExternalSort::merge(std::array<ulong, HELP_FILES_COUNT> &currentFilesLength) {
    std::array<ulong, HELP_FILES_COUNT> targetFilesLength{0}, currentFilesIndices{0};
    auto targetFilesLengthIterator = targetFilesLength.begin();
    std::for_each(currentFiles->begin(), currentFiles->end(), [](Stream &file){
        file.seekp(0);
    });
    FileElement prevElement{0, INVALID_INDEX};
    auto activeTargetFileIterator = targetFiles->begin();
    bool isSorted{false}, heapFull{false};
    std::array<bool, HELP_FILES_COUNT> seriesEnded{false};
    uint sortedFile;

    do {
        if(!heapFull) {
            if(prevElement.index != INVALID_INDEX) {
                readNext(currentFilesIndices, currentFilesLength, prevElement, seriesEnded);
            } else {
                for(int i = 0; i < HELP_FILES_COUNT; i++) {
                    prevElement.index = i;
                    readNext(currentFilesIndices, currentFilesLength, prevElement, seriesEnded);
                }
            }
        }
        if(activeHeap->empty()) {
            if(sideHeap->empty()) {
                std::swap(currentFiles, targetFiles);
                currentFilesLength = targetFilesLength;
                targetFilesLength.fill(0);
                currentFilesIndices.fill(0);
                activeTargetFileIterator = targetFiles->begin();
                targetFilesLengthIterator = targetFilesLength.begin();
                prevElement = {0, INVALID_INDEX};
                std::for_each(currentFiles->begin(), currentFiles->end(), [](Stream &file){
                    file.seekg(0);
                });
                std::for_each(targetFiles->begin(), targetFiles->end(), [](Stream &file){
                    file.seekp(0);
                });
                heapFull = false;
                isSorted = checkOneSeriesLeft(currentFilesLength, sortedFile);
            } else {
                advanceIterator(activeTargetFileIterator, *targetFiles);
                advanceIterator(targetFilesLengthIterator, targetFilesLength);
                std::swap(activeHeap, sideHeap);
                heapFull = true;
            }
            seriesEnded.fill(false);
        } else {
            prevElement = activeHeap->top();
            activeTargetFileIterator->write(reinterpret_cast<char *>(&prevElement.value), sizeof(prevElement.value));
            activeHeap->pop();
            (*targetFilesLengthIterator)++;
            heapFull = false;
        }
    } while(!isSorted);
    return sortedFile;
}

void ExternalSort::terminateFiles() {
    for(uint i = 0; i < HELP_FILES_COUNT; i++) {
        (*currentFiles)[i].close();
        (*targetFiles)[i].close();
    }
}

ulong ExternalSort::getFileDivisionBytes(uint countFileParts) {
    double numCount = static_cast<double>(getFileSize(A)) / static_cast<double>(sizeof(uint));
    return static_cast<ulong>(ceil(numCount / static_cast<double>(countFileParts))) * sizeof(uint);
}

void ExternalSort::writeDestFile(uint sortedFileIndex, const char *destFile) {
    std::string sortedFile = (*currentFiles)[sortedFileIndex].getPath();
    const char *srcFile = sortedFile.c_str();
    std::filesystem::rename(srcFile, destFile);
}