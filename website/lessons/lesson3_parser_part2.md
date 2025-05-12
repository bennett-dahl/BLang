# Lesson 3: Syntax Analysis (Part 2) - Implementing the Parser

## 3. Implementing the Parser with Bison

Now that we've defined our AST structure, let's implement the parser using Bison. Bison is a parser generator that works well with Flex to create a complete frontend for our language.

### 3.1 Creating the Bison Grammar File

The Bison grammar file defines the syntax rules of our language and specifies how to build AST nodes when those rules are recognized. We would create a file named `src/parser.y` that contains:

- Token definitions matching what our lexer produces
- Grammar rules describing valid BLang syntax
- Actions to construct AST nodes for each grammar rule
- Operator precedence and associativity rules

### 3.2 Creating the Parser Interface

We'll need a parser interface that uses the Bison-generated parser to build an AST from the source code:

```cpp
#ifndef BLANG_PARSER_H
#define BLANG_PARSER_H

#include "AST.h"
#include <string>
#include <memory>

namespace BLang {

class Parser {
private:
    std::string sourceCode;

public:
    Parser(const std::string& sourceCode);

    // Parse the source code and return the AST
    std::shared_ptr<Program> parse();

    // Print the AST to the given output stream
    void printAST(std::ostream& os, std::shared_ptr<Program> program);
};

} // namespace BLang

#endif // BLANG_PARSER_H
```

### 3.3 Integrating Bison with CMake

We need to update our CMake configuration to handle Bison-generated files alongside Flex:

```cmake
# Find Flex and Bison
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# Generate lexer from Flex file
flex_target(Lexer src/lexer.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.cpp)

# Generate parser from Bison file
bison_target(Parser src/parser.y ${CMAKE_CURRENT_BINARY_DIR}/parser.cpp)

# Add the lexer and parser dependency
add_flex_bison_dependency(Lexer Parser)
```

## 4. Common Parsing Challenges

When implementing a parser for an object-oriented language, you'll encounter several challenges:

### 4.1 Resolving Grammar Ambiguities

The classic "dangling else" problem occurs when it's unclear which `if` statement an `else` clause belongs to. Bison's default behavior associates each `else` with the nearest `if`.

Operator precedence ambiguities are resolved by explicitly specifying precedence and associativity in the Bison file.

### 4.2 Error Recovery

A robust parser should continue parsing even after encountering syntax errors. Adding error recovery productions helps:

```
statement: expression SEMICOLON
         | error SEMICOLON { yyerrok; }
         ;
```

## 5. Next Steps

With our lexer and parser implementation, we can convert BLang source code into an AST. In the next lesson, we'll perform semantic analysis to:

1. Build a symbol table for variables, functions, and classes
2. Implement type checking and semantic validation
3. Handle inheritance and method overriding validation
4. Resolve variable references across different scopes

## Assignment

1. Complete the parser implementation by:

   - Writing the full Bison grammar for BLang
   - Implementing error recovery mechanisms
   - Creating comprehensive test cases

2. Extend the grammar to support additional features:
   - Array types and array access expressions
   - Multiple variable declarations in a single statement
   - Enhanced object-oriented features like interfaces

## Resources

- [Bison Manual](https://www.gnu.org/software/bison/manual/)
- [Compilers: Principles, Techniques, and Tools](https://www.amazon.com/Compilers-Principles-Techniques-Tools-2nd/dp/0321486811) (Chapter 4: Syntax Analysis)
