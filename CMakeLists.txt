cmake_minimum_required(VERSION 3.15)
project(EmulatorTest C)

set(CMAKE_C_STANDARD 99)

add_executable(EmulatorTest main.c cpu.c cpu.h utilities.c MyMemory.c MyMemory.h cpu.h)