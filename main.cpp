#include <iostream>
#include <fstream>
#include "Random.h"
#include "StreamException.h"
#include "Error.h"
#include "ExternalSort.h"
#include <chrono>
#include <queue>
#include <cstring>
#include <windows.h>

static constexpr int TARGET_ARG_COUNT{6};
static constexpr const char *ORDINARY_SORT_FLAG{"-o"};
static constexpr const char *MODIFIED_SORT_FLAG{"-m"};
static constexpr const char *MEGABYTES_FLAG{"-mb"};
static constexpr const char *GIGABYTES_FLAG{"-gb"};

void genRandomFile(const char *path, ulong bytesCount, uint upperLimit);
void handleArgCount(int actualArgCount, int targetArgCount);
void limitRAMUsage(ulong bytesCount);

int main(int argc, char **argv) {
    handleArgCount(argc, TARGET_ARG_COUNT);
    const char *size = argv[1];
    ulong fileSize = _atoi64(size);
    const char *sizeMeasureFlag = argv[2];
    const char *targetFile = argv[3];
    const char *destFile = argv[4];
    const char *sortFlag = argv[5];
    if(strcmp(sizeMeasureFlag, MEGABYTES_FLAG) == 0) {
        fileSize = mb_to_b(fileSize);
    } else if(strcmp(sizeMeasureFlag, GIGABYTES_FLAG) == 0) {
        fileSize = gb_to_b(fileSize);
    }
    genRandomFile(targetFile, fileSize, 1000000);
    ulong maxBytesUsage = mb_to_b(512);
    limitRAMUsage(maxBytesUsage);
    void (ExternalSort::*sortFunction)(const char *, const char *) = nullptr;
    if(strcmp(sortFlag, ORDINARY_SORT_FLAG) == 0) {
        sortFunction = &ExternalSort::nWayMergeSort;
    } else if(strcmp(sortFlag, MODIFIED_SORT_FLAG) == 0) {
        sortFunction = &ExternalSort::modifiedNWayMergeSort;
    } else {
        Error::fallWithError("ARG::INAPPROPRIATE_FLAG::<-n><-o>");
    }
    ExternalSort alg{};
    auto timeStart = std::chrono::high_resolution_clock::now();
    (alg.*sortFunction)(targetFile, destFile);
    auto timeFinish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = timeFinish - timeStart;
    std::cout << "Time taken: " << duration.count() << " seconds\n";
    std::cout << "Any key to exit\n";
    getchar();
    return 0;
}

void genRandomFile(const char *path, ulong bytesCount, uint upperLimit) {
    Random rand;
    std::vector<uint> randomArray = rand.genRandomNumbers(bytesCount, upperLimit);
    try {
        std::ofstream os{path, std::ios::binary};
        if(!os) {
            throw StreamException{"FAILED_TO_OPEN"};
        }
        os.write(reinterpret_cast<const char *>(randomArray.data()),
                 static_cast<long long>(randomArray.size() * sizeof(uint)));
        if(os.bad()) {
            throw StreamException{"FAILED_TO_WRITE"};
        }
        os.close();
    } catch(StreamException &ex) {
        Error::fallWithError(ex.what());
    }
}

void handleArgCount(int actualArgCount, int targetArgCount) {
    if(actualArgCount != targetArgCount) {
        Error::fallWithError("ARG::<target> <destination> <algorithm>");
    }
}

void limitRAMUsage(ulong bytesCount) {
    HANDLE hJob = CreateJobObject(nullptr, nullptr);
    if (hJob == nullptr) {
        Error::fallWithError("MEMORY::FAILED_TO_CREATE_JOB_OBJECT");
    }

    JOBOBJECT_EXTENDED_LIMIT_INFORMATION jobLimit = {};
    jobLimit.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_PROCESS_MEMORY;
    jobLimit.ProcessMemoryLimit = bytesCount;

    if (!SetInformationJobObject(hJob, JobObjectExtendedLimitInformation,
                                 &jobLimit, sizeof(jobLimit))) {
        Error::fallWithError("MEMORY::FAILED_TO_SET_INFO_JOB_OBJECT");
    }

    if (!AssignProcessToJobObject(hJob, GetCurrentProcess())) {
        Error::fallWithError("MEMORY::FAILED_TO_ASSIGN_PROCESS");
    }
}