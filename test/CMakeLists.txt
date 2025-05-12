# Test CMakeLists.txt

# Lexer tests
add_executable(lexer_tests lexer_tests.cpp)
target_link_libraries(lexer_tests blang_lib)
add_test(NAME LexerTests COMMAND lexer_tests) 