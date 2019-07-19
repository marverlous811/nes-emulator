cmake_minimum_required(VERSION 2.8.3)
project(nes_emulator)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${nes_emulator_SOURCE_DIR}/cmake")
set(BIN_DIR ${nes_emulator_SOURCE_DIR}/bin)

# Bump up warning levels appropriately for clang, gcc & msvc and build in debug mode
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -std=c++11 -Wno-gnu-zero-variadic-macro-arguments")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2")
elseif (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  if (CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
  endif()
endif()

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

include_directories(src)
file(GLOB_RECURSE NES src/nes/*.cpp src/nes/*.hpp src/nes/memory_component/*)
file(GLOB_RECURSE util src/util/*cpp src/util/*.h src/util/*.hpp)

SET(TEST src/test/cpuTest.cpp)

ADD_EXECUTABLE(nes_emulator src/main.cpp ${NES} ${util})
ADD_EXECUTABLE(test ${TEST} ${NES} ${util})

target_link_libraries(nes_emulator ${SDL2_LIBRARY})
install(TARGETS nes_emulator RUNTIME DESTINATION ${BIN_DIR})

target_link_libraries(test ${SDL2_LIBRARY})
install(TARGETS test RUNTIME DESTINATION ${BIN_DIR})