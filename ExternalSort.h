#pragma once

#include <vector>
#include <array>
#include <fstream>
#include <queue>
#include "Stream.h"
#include "utils.h"
#include "FileElement.h"
#include "FileElementComparator.h"

class ExternalSort {
private:
    static constexpr uint HELP_FILES_COUNT{3};
    static constexpr int INVALID_INDEX{-1};
    static constexpr uint COUNT_FILE_PARTS{10};
    static const std::string DIRECTORY;
    static const std::string EXTENSION;
    const char *errorLog;
    bool errorState;
    std::ifstream A;
    std::array<Stream, HELP_FILES_COUNT> *currentFiles;
    std::array<Stream, HELP_FILES_COUNT> *targetFiles;
    std::priority_queue<FileElement, std::vector<FileElement>, FileElementComparator> *activeHeap;
    std::priority_queue<FileElement, std::vector<FileElement>, FileElementComparator> *sideHeap;

    void createHelpFiles(const char *path);
    void readNext(std::array<ulong, HELP_FILES_COUNT> &currentFilesIndices,
                  std::array<ulong, HELP_FILES_COUNT> currentFilesLength,
                  const FileElement &prevElement,
                  std::array<bool, HELP_FILES_COUNT> &seriesEnded);
    uint merge(std::array<ulong, HELP_FILES_COUNT> &currentFilesLength);
    void terminateFiles();
    void writeDestFile(uint sortedFileIndex, const char *destFile);
    ulong getFileDivisionBytes(uint countFileParts);
    static bool checkOneSeriesLeft(const std::array<ulong, HELP_FILES_COUNT> &currentFilesLength, uint &destFile);
public:
    ExternalSort();
    void nWayMergeSort(const char *path, const char *dest);
    void modifiedNWayMergeSort(const char *path, const char *dest);
    NODISCARD bool fail() const;
    NODISCARD const char *getErrorLog() const;
    ~ExternalSort();
};