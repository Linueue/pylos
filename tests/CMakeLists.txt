cmake_minimum_required(VERSION 3.20)

include(Catch)

add_executable(lexer_tests lexer_test.cpp)
target_link_libraries(lexer_tests PRIVATE lexer_lib Catch2::Catch2WithMain)
catch_discover_tests(lexer_tests)
