cmake_minimum_required(VERSION 3.29)
project(nway_external_sort)

set(CMAKE_CXX_STANDARD 20)

add_executable(nway_external_sort
        main.cpp
        Error.h
        Error.cpp
        ExternalSort.h
        ExternalSort.cpp
        FileElement.h
        FileElementComparator.h
        FileElementComparator.cpp
        Random.h
        Random.cpp
        Stream.h
        Stream.cpp
        StreamException.h
        StreamException.cpp
        utils.h
        utils.cpp
)