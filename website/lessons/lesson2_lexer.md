# Lesson 2: Lexical Analysis

## Learning Objectives

- Understand the principles of lexical analysis and tokenization
- Implement a lexer using Flex
- Define token types for an object-oriented language
- Test the lexer with simple inputs

## 1. Understanding Lexical Analysis

Lexical analysis is the first phase of a compiler or interpreter. It takes the raw source code as input and produces a sequence of tokens as output. A token is a string with an assigned and thus identified meaning, like a keyword, identifier, or operator.

### 1.1 The Role of a Lexer

The lexer's primary responsibilities include:

1. **Reading the input stream**: Processing source code character by character
2. **Recognizing lexemes**: Identifying meaningful character sequences (lexemes)
3. **Categorizing lexemes**: Assigning token types to lexemes
4. **Handling lexical errors**: Detecting invalid lexemes
5. **Managing source locations**: Tracking line and column numbers for error reporting
6. **Ignoring whitespace and comments**: Filtering non-essential characters

### 1.2 Regular Expressions and Finite Automata

Lexers typically use regular expressions to define token patterns. These regular expressions can be converted into finite automata (state machines) that efficiently recognize tokens:

- **Regular expressions**: Patterns that describe sets of strings
- **Deterministic Finite Automata (DFA)**: State machines that recognize regular languages
- **Nondeterministic Finite Automata (NFA)**: More flexible state machines that can be converted to DFAs

Tools like Flex automatically convert regular expressions into efficient DFAs for token recognition.

## 2. Setting Up Flex for BLang

Flex (Fast Lexical Analyzer Generator) will help us generate a high-performance lexer from a set of regular expressions. Let's set up Flex for our BLang language:

### 2.1 Creating the Flex Input File

First, create a `src/lexer.l` file with the following structure:

```flex
%{
/* C++ code section: includes, declarations, etc. */
#include "Token.h"
#include <string>
#include <vector>
#include <iostream>

namespace BLang {
    // Forward declarations
    static TokenType keywordType(const std::string& lexeme);
    static bool isKeyword(const std::string& lexeme);
}

// Globals for tracking position
static int line = 1;
static int column = 1;
static std::vector<BLang::Token> tokens;

// Helper function to update column
static void updateColumn(int length) {
    column += length;
}

// Helper function to update line
static void newLine() {
    line++;
    column = 1;
}

%}

/* Flex options */
%option noyywrap
%option c++
%option yyclass="BLang::Lexer"
%option prefix="BLang"

/* Define named patterns */
DIGIT       [0-9]
LETTER      [a-zA-Z]
IDENTIFIER  {LETTER}({LETTER}|{DIGIT}|_)*
INTEGER     {DIGIT}+
FLOAT       {DIGIT}+\.{DIGIT}+([eE][+-]?{DIGIT}+)?
WHITESPACE  [ \t\r]+
NEWLINE     \n
STRING      \"([^\"\n\\]|\\['"?\\abfnrtv])*\"
COMMENT     \/\/[^\n]*
MLCOMMENT   \/\*([^*]|\*+[^*/])*\*+\/

%%

{WHITESPACE}    { updateColumn(yyleng); /* Skip whitespace */ }
{NEWLINE}       { newLine(); /* Update line counting */ }
{COMMENT}       { updateColumn(yyleng); /* Skip single-line comments */ }
{MLCOMMENT}     { /* Handle multi-line comments */
                  for(int i = 0; i < yyleng; i++) {
                      if(yytext[i] == '\n') newLine();
                      else column++;
                  }
                }

"class"         { updateColumn(yyleng); return BLang::TokenType::CLASS; }
"extends"       { updateColumn(yyleng); return BLang::TokenType::EXTENDS; }
"public"        { updateColumn(yyleng); return BLang::TokenType::PUBLIC; }
"private"       { updateColumn(yyleng); return BLang::TokenType::PRIVATE; }
"protected"     { updateColumn(yyleng); return BLang::TokenType::PROTECTED; }
"void"          { updateColumn(yyleng); return BLang::TokenType::VOID; }
"int"           { updateColumn(yyleng); return BLang::TokenType::INT; }
"float"         { updateColumn(yyleng); return BLang::TokenType::FLOAT; }
"boolean"       { updateColumn(yyleng); return BLang::TokenType::BOOLEAN; }
"string"        { updateColumn(yyleng); return BLang::TokenType::STRING; }
"var"           { updateColumn(yyleng); return BLang::TokenType::VAR; }
"if"            { updateColumn(yyleng); return BLang::TokenType::IF; }
"else"          { updateColumn(yyleng); return BLang::TokenType::ELSE; }
"while"         { updateColumn(yyleng); return BLang::TokenType::WHILE; }
"for"           { updateColumn(yyleng); return BLang::TokenType::FOR; }
"return"        { updateColumn(yyleng); return BLang::TokenType::RETURN; }
"new"           { updateColumn(yyleng); return BLang::TokenType::NEW; }
"this"          { updateColumn(yyleng); return BLang::TokenType::THIS; }
"super"         { updateColumn(yyleng); return BLang::TokenType::SUPER; }
"true"          { updateColumn(yyleng); return BLang::TokenType::BOOLEAN_LITERAL; }
"false"         { updateColumn(yyleng); return BLang::TokenType::BOOLEAN_LITERAL; }
"null"          { updateColumn(yyleng); return BLang::TokenType::NULL_LITERAL; }

{IDENTIFIER}    {
                  updateColumn(yyleng);
                  std::string lexeme(yytext, yyleng);
                  if (isKeyword(lexeme)) {
                      return keywordType(lexeme);
                  } else {
                      return BLang::TokenType::IDENTIFIER;
                  }
                }

{INTEGER}       { updateColumn(yyleng); return BLang::TokenType::INTEGER_LITERAL; }
{FLOAT}         { updateColumn(yyleng); return BLang::TokenType::FLOAT_LITERAL; }
{STRING}        { updateColumn(yyleng); return BLang::TokenType::STRING_LITERAL; }

"="             { updateColumn(yyleng); return BLang::TokenType::ASSIGN; }
"+"             { updateColumn(yyleng); return BLang::TokenType::PLUS; }
"-"             { updateColumn(yyleng); return BLang::TokenType::MINUS; }
"*"             { updateColumn(yyleng); return BLang::TokenType::STAR; }
"/"             { updateColumn(yyleng); return BLang::TokenType::SLASH; }
"%"             { updateColumn(yyleng); return BLang::TokenType::PERCENT; }
"=="            { updateColumn(yyleng); return BLang::TokenType::EQUAL; }
"!="            { updateColumn(yyleng); return BLang::TokenType::NOT_EQUAL; }
"<"             { updateColumn(yyleng); return BLang::TokenType::LESS; }
"<="            { updateColumn(yyleng); return BLang::TokenType::LESS_EQUAL; }
">"             { updateColumn(yyleng); return BLang::TokenType::GREATER; }
">="            { updateColumn(yyleng); return BLang::TokenType::GREATER_EQUAL; }
"&&"            { updateColumn(yyleng); return BLang::TokenType::AND; }
"||"            { updateColumn(yyleng); return BLang::TokenType::OR; }
"!"             { updateColumn(yyleng); return BLang::TokenType::NOT; }

"."             { updateColumn(yyleng); return BLang::TokenType::DOT; }
","             { updateColumn(yyleng); return BLang::TokenType::COMMA; }
";"             { updateColumn(yyleng); return BLang::TokenType::SEMICOLON; }
":"             { updateColumn(yyleng); return BLang::TokenType::COLON; }
"("             { updateColumn(yyleng); return BLang::TokenType::LEFT_PAREN; }
")"             { updateColumn(yyleng); return BLang::TokenType::RIGHT_PAREN; }
"{"             { updateColumn(yyleng); return BLang::TokenType::LEFT_BRACE; }
"}"             { updateColumn(yyleng); return BLang::TokenType::RIGHT_BRACE; }

.               { updateColumn(yyleng); return BLang::TokenType::ERROR; }

%%

namespace BLang {

/* Implementation of keywordType function */
static TokenType keywordType(const std::string& lexeme) {
    static const std::unordered_map<std::string, TokenType> keywords = {
        {"class", TokenType::CLASS},
        {"extends", TokenType::EXTENDS},
        {"public", TokenType::PUBLIC},
        {"private", TokenType::PRIVATE},
        {"protected", TokenType::PROTECTED},
        {"void", TokenType::VOID},
        {"int", TokenType::INT},
        {"float", TokenType::FLOAT},
        {"boolean", TokenType::BOOLEAN},
        {"string", TokenType::STRING},
        {"var", TokenType::VAR},
        {"if", TokenType::IF},
        {"else", TokenType::ELSE},
        {"while", TokenType::WHILE},
        {"for", TokenType::FOR},
        {"return", TokenType::RETURN},
        {"new", TokenType::NEW},
        {"this", TokenType::THIS},
        {"super", TokenType::SUPER},
        {"true", TokenType::BOOLEAN_LITERAL},
        {"false", TokenType::BOOLEAN_LITERAL},
        {"null", TokenType::NULL_LITERAL}
    };

    auto it = keywords.find(lexeme);
    if (it != keywords.end()) {
        return it->second;
    }
    return TokenType::IDENTIFIER;
}

/* Implementation of isKeyword function */
static bool isKeyword(const std::string& lexeme) {
    return keywordType(lexeme) != TokenType::IDENTIFIER;
}

} // namespace BLang
```

### 2.2 Integrating with Your Project

After creating the Flex file, you'll need to:

1. **Generate the C++ code**: Use Flex to generate the lexer C++ code:

```bash
# From the project root directory
flex -o src/lexer.cpp src/lexer.l
```

2. **Create Token Header File**: Create an `include/Token.h` header file to define the token types:

```cpp
// Token.h
#ifndef BLANG_TOKEN_H
#define BLANG_TOKEN_H

#include <string>
#include <variant>
#include <unordered_map>

namespace BLang {

enum class TokenType {
    // Keywords
    CLASS, EXTENDS, PUBLIC, PRIVATE, PROTECTED,
    VOID, INT, FLOAT, BOOLEAN, STRING, VAR,
    IF, ELSE, WHILE, FOR, RETURN, NEW, THIS, SUPER,

    // Literals
    INTEGER_LITERAL, FLOAT_LITERAL, STRING_LITERAL,
    BOOLEAN_LITERAL, NULL_LITERAL,

    // Identifiers
    IDENTIFIER,

    // Operators
    ASSIGN, PLUS, MINUS, STAR, SLASH, PERCENT,
    EQUAL, NOT_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
    AND, OR, NOT,

    // Punctuation
    DOT, COMMA, SEMICOLON, COLON,
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,

    // Special
    EOF_TOKEN, ERROR
};

// Token class
class Token {
private:
    TokenType type;
    std::string lexeme;
    int line;
    int column;

public:
    Token(TokenType type, const std::string& lexeme, int line, int column)
        : type(type), lexeme(lexeme), line(line), column(column) {}

    TokenType getType() const { return type; }
    std::string getLexeme() const { return lexeme; }
    int getLine() const { return line; }
    int getColumn() const { return column; }

    std::string toString() const;
};

} // namespace BLang

#endif // BLANG_TOKEN_H
```

3. **Create Lexer Header File**: Create an `include/Lexer.h` header file:

```cpp
// Lexer.h
#ifndef BLANG_LEXER_H
#define BLANG_LEXER_H

#include "Token.h"
#include <string>
#include <vector>

namespace BLang {

class Lexer {
private:
    std::string source;
    std::vector<Token> tokens;

public:
    Lexer(const std::string& source);
    std::vector<Token> scanTokens();
};

} // namespace BLang

#endif // BLANG_LEXER_H
```

4. **Update CMakeLists.txt**: Add the lexer files to your build:

```cmake
# In your main CMakeLists.txt
add_library(blang_lib
    src/lexer.cpp
    src/token.cpp
    # Other source files...
)
```

5. **Development Workflow**:

   - Edit the lexer.l file to define token patterns
   - Run flex to generate lexer.cpp
   - Build the project using cmake
   - Test the lexer with sample inputs

### 2.3 C++ Implementation Details

When implementing your lexer, follow these best practices:

1. **Namespace**: Keep all BLang code in the `BLang` namespace
2. **Error Handling**: Track line and column for accurate error messages
3. **Memory Management**: Use modern C++ features (std::string, std::vector) to avoid manual memory management
4. **Testing**: Write test cases to verify the lexer handles all valid tokens and edge cases

### 2.4 Building and Testing the Lexer

To build and test your lexer, follow these steps:

```bash
# From the project root
mkdir -p build
cd build
cmake ..
cmake --build .

# Run tests
./test/lexer_tests

# Test with a sample input
./blang ../examples/test.bl
```

## 3. Token Types for BLang

// ... existing code ...
