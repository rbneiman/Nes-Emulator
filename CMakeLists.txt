cmake_minimum_required(VERSION 3.15)
project(EmulatorTest)

set(CMAKE_CXX_STANDARD 14)
set(SFML_DIR "SFML-2.5.1/lib/cmake/SFML")

find_package(SFML 2 COMPONENTS graphics audio REQUIRED)
add_executable(EmulatorTest main.cpp cpu.cpp cpu.h utilities.cpp memory.cpp memory.h cpu.h ppu.cpp ppu.h screen.cpp screen.h)
target_link_libraries(EmulatorTest sfml-graphics sfml-audio)