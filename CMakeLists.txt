cmake_minimum_required(VERSION 2.8.7)
project(mpi-parser)

if(NOT ${CMAKE_VERSION} LESS 3.2)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
    message(STATUS "Checking compiler flags for C++11 support.")
    # Set C++11 support flags for various compilers
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
    check_cxx_compiler_flag("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
    if(COMPILER_SUPPORTS_CXX11)
        message(STATUS "C++11 is supported.")
        if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
        endif()
    elseif(COMPILER_SUPPORTS_CXX0X)
        message(STATUS "C++0x is supported.")
        if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -stdlib=libc++")
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
        endif()
    else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
    endif()
endif()

find_library(gumbo REQUIRED)
find_package(Boost REQUIRED mpi serialization)
find_package(MPI REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${MPI_CXX_INCLUDE_PATH})
add_subdirectory(opt)

add_executable(mpi-parser main-mpi.cpp json.hpp)
add_executable(seq-parser main-seq.cpp json.hpp)
target_link_libraries(mpi-parser gumbo ${CPR_LIBRARIES} ${CURL_LIBRARIES} ${Boost_LIBRARIES} ${MPI_CXX_LIBRARIES})
target_link_libraries(seq-parser gumbo ${CPR_LIBRARIES} ${CURL_LIBRARIES})
include_directories(${CPR_INCLUDE_DIRS} ${JSON_INCLUDE_DIRS})
