cmake_minimum_required(VERSION 3.15)
project(EmulatorTest)
set(CMAKE_CXX_STANDARD 14)

include(FetchContent)
FetchContent_Declare(
    sfml
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2f11710abc5aa478503a7ff3f9e654bd2078ebab # 2.5.1
)
FetchContent_MakeAvailable(sfml)
FetchContent_GetProperties(sfml)

add_executable(EmulatorTest
        main.cpp
        cpu.cpp
        ppu.cpp
        utilities.cpp
        memory.cpp
        screen.cpp
        rom.cpp
        NESSystem.cpp
        Input.cpp
        Mapper.cpp)
target_link_libraries(EmulatorTest PRIVATE
        sfml-graphics
        sfml-audio)

#copy the built library files to the same directory as the executable
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${sfml_BINARY_DIR}/lib
        ${CMAKE_CURRENT_BINARY_DIR})
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
