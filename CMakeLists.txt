cmake_minimum_required(VERSION 3.20)

project(Pylos VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES_CPP "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADERS_H "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
list(REMOVE_ITEM SOURCES_CPP "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_library(pylos_lib ${SOURCES_CPP})
target_include_directories(pylos_lib PUBLIC src/)

Include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.16.1
)
FetchContent_MakeAvailable(Catch2)

enable_testing()
add_subdirectory(tests)

add_executable(Pylos "src/main.cpp")
target_link_libraries(Pylos PRIVATE pylos_lib)
