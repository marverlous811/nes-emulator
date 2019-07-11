cmake_minimum_required(VERSION 3.13)
project(nes_emulator)

if(NOT MSVC)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
    if(COMPILER_SUPPORTS_CXX11)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -g")
    else()
        message(FATAL_ERROR "could not find C++11 capable compiler")
    endif()
endif()

include_directories(src)
file(GLOB_RECURSE NES src/nes/*.cpp src/nes/*.hpp)

SET(TEST src/test/cpuTest.cpp)

ADD_EXECUTABLE(exec src/main.cpp ${NES})
ADD_EXECUTABLE(test ${TEST} ${NES})