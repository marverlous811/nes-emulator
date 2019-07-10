cmake_minimum_required(VERSION 3.13)
project(nes_emulator)

SET(NES_CPU src/nes/cpu/cpu.cpp src/nes/cpu/ram.cpp src/nes/cpu/cpu_mmu.cpp)
SET(NES_LOAD src/nes/loadRom/Cartridge.cpp src/nes/loadRom/INes.cpp)
SET(NES_MAPPER src/nes/mapper/mapper.cpp src/nes/mapper/mapper_000.cpp)
SET(NES src/nes/nes.cpp ${NES_LOAD} ${NES_MAPPER} ${NES_CPU})
SET(TEST src/test/cpuTest.cpp)

ADD_EXECUTABLE(exec src/main.cpp ${NES})
ADD_EXECUTABLE(test ${TEST} ${NES})