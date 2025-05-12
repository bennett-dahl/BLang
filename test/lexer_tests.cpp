#include <iostream>
#include <cassert>
#include <blang/lexer.h>

// Simple test framework
#define TEST(name)                              \
    void name();                                \
    std::cout << "Running " << #name << "... "; \
    name();                                     \
    std::cout << "PASSED\n"

void test_lexer_creation()
{
    blang::Lexer lexer;
    // Just testing that we can create a lexer instance
    assert(true);
}

void test_empty_input()
{
    blang::Lexer lexer;
    auto tokens = lexer.tokenize("");
    assert(tokens.size() == 1);
    assert(tokens[0].type == blang::Token::Type::EOF_TOKEN);
}

int main()
{
    std::cout << "Running lexer tests...\n";

    TEST(test_lexer_creation);
    TEST(test_empty_input);

    std::cout << "All tests passed!\n";
    return 0;
}