cmake_minimum_required(VERSION 3.20)

project(Pylos VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

if(EMSCRIPTEN)
    message(STATUS "Configuring for Emscripten (Wasm)")
else()
    message(STATUS "Configuring for native ${CMAKE_SYSTEM_NAME}")
endif()

file(GLOB_RECURSE SOURCES_CPP "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
list(REMOVE_ITEM SOURCES_CPP
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/wasm/wasm_exports.cpp"
)

add_library(pylos_lib ${SOURCES_CPP})
target_include_directories(pylos_lib PUBLIC src/)

if(NOT EMSCRIPTEN)
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
endif()

if(EMSCRIPTEN)
    add_executable(PylosWasm "src/wasm/wasm_exports.cpp")
    target_link_libraries(PylosWasm PRIVATE pylos_lib)

    target_link_options(PylosWasm PRIVATE
        "-sMODULARIZE=1"
        "-sEXPORT_NAME=createPylosModule"
        "-sALLOW_MEMORY_GROWTH=1"
        "-sEXPORTED_FUNCTIONS=['_lexer_create', '_lexer_next', '_lexer_destroy', '_malloc', '_free']"
        "-sEXPORTED_RUNTIME_METHODS=['ccall', 'cwrap', 'UTF8ToString', 'stringToUTF8', 'lengthBytesUTF8', 'getValue']"
    )
    set_target_properties(PylosWasm PROPERTIES
        OUTPUT_NAME "pylos"
        SUFFIX ".js"
    )
endif()
