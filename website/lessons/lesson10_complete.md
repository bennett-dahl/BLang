# Lesson 10: Bringing It All Together

## Learning Objectives

- Integrate all components of BLang into a cohesive system
- Create a command-line interface for the language
- Package the compiler/interpreter for distribution
- Develop sample BLang programs to demonstrate language features
- Learn how to extend BLang with new features

## 1. Building the Complete BLang System

Throughout this course, we've built each component of our object-oriented programming language. Now it's time to integrate everything into a complete language implementation.

### 1.1 System Architecture

Let's review the architecture of our complete BLang system:

```
┌─────────────────┐      ┌─────────────────┐      ┌─────────────────┐
│  Source Code    │ ──► │    Lexer        │ ──► │    Parser       │
└─────────────────┘      └─────────────────┘      └─────────────────┘
                                                          │
                                                          ▼
┌─────────────────┐      ┌─────────────────┐      ┌─────────────────┐
│  Virtual        │ ◄── │  IR Generator   │ ◄── │  Type Checker   │
│  Machine        │      └─────────────────┘      └─────────────────┘
└─────────────────┘
        │
        ▼
┌─────────────────┐
│  Program        │
│  Output         │
└─────────────────┘
```

Our language pipeline consists of the following components:

1. **Lexer**: Tokenizes source code
2. **Parser**: Builds abstract syntax tree
3. **Type Checker**: Verifies type correctness and builds symbol table
4. **IR Generator**: Translates AST to intermediate representation
5. **Virtual Machine**: Executes the IR code

### 1.2 Main Compiler Driver

Let's create the main driver for our language:

```cpp
#include "Lexer/Lexer.h"
#include "Parser/Parser.h"
#include "TypeChecker/TypeChecker.h"
#include "IR/IRGenerator.h"
#include "VM/VirtualMachine.h"
#include "StandardLibrary/StandardLibrary.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

namespace BLang {

class Compiler {
public:
    // Compile and run a file
    int compileAndRunFile(const std::string& filename) {
        // Read the file
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Error: Could not open file " << filename << std::endl;
            return 1;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string sourceCode = buffer.str();

        return compileAndRunCode(sourceCode, filename);
    }

    // Compile and run a string
    int compileAndRunCode(const std::string& sourceCode, const std::string& sourceFilename) {
        try {
            // Stage 1: Lexical analysis
            Lexer lexer(sourceCode, sourceFilename);
            std::vector<Token> tokens = lexer.tokenize();

            // Stage 2: Parsing
            Parser parser(tokens);
            std::shared_ptr<AST::Program> ast = parser.parseProgram();

            // Stage 3: Semantic analysis and type checking
            TypeChecker typeChecker;
            typeChecker.check(ast);

            // Stage 4: IR generation
            IRGenerator irGenerator;
            std::shared_ptr<IR::Module> irModule = irGenerator.generate(ast);

            // Stage 5: Execution
            VirtualMachine vm(irModule);

            // Initialize standard library
            StandardLibrary::initialize(vm);

            // Initialize VM
            vm.initialize();

            // Run the program
            vm.run();

            return 0;
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    }

    // Run interactive REPL
    void runRepl() {
        std::cout << "BLang Interactive Shell (REPL)" << std::endl;
        std::cout << "Type '.exit' to quit" << std::endl;

        std::string line;
        while (true) {
            std::cout << ">>> ";
            if (!std::getline(std::cin, line)) {
                break;
            }

            if (line == ".exit") {
                break;
            }

            // Execute the line
            compileAndRunCode(line, "<repl>");
        }
    }
};

} // namespace BLang

// Main entry point
int main(int argc, char* argv[]) {
    BLang::Compiler compiler;

    if (argc < 2) {
        // No arguments, run REPL
        compiler.runRepl();
        return 0;
    }

    std::string arg1 = argv[1];
    if (arg1 == "-h" || arg1 == "--help") {
        std::cout << "Usage: blang [options] [script]" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -h, --help     Show this help message" << std::endl;
        std::cout << "  -v, --version  Show version information" << std::endl;
        std::cout << "  -i, --interactive  Run in interactive mode (REPL)" << std::endl;
        return 0;
    }
    else if (arg1 == "-v" || arg1 == "--version") {
        std::cout << "BLang version 1.0.0" << std::endl;
        return 0;
    }
    else if (arg1 == "-i" || arg1 == "--interactive") {
        compiler.runRepl();
        return 0;
    }
    else {
        // Treat as a script file
        return compiler.compileAndRunFile(arg1);
    }
}
```

## 2. Building and Packaging BLang

### 2.1 CMake Build System

Let's set up a CMake build system to compile and package BLang:

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(BLang VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
    src/main.cpp
    src/Lexer/Lexer.cpp
    src/Parser/Parser.cpp
    src/TypeChecker/TypeChecker.cpp
    src/IR/IRGenerator.cpp
    src/VM/VirtualMachine.cpp
    src/VM/MemoryManager.cpp
    src/StandardLibrary/StandardLibrary.cpp
)

# Add the executable
add_executable(blang ${SOURCES})

# Include directories
target_include_directories(blang PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Install targets
install(TARGETS blang DESTINATION bin)
install(FILES README.md DESTINATION share/blang)
install(DIRECTORY examples/ DESTINATION share/blang/examples)

# Packaging
include(CPack)
```

### 2.2 Creating a Distribution Package

To create a distributable package for BLang, we'll use CMake's CPack:

```cmake
# Add to CMakeLists.txt
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "BLang - A modern object-oriented programming language")
set(CPACK_PACKAGE_VENDOR "Your Name")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

# Create packages
set(CPACK_GENERATOR "TGZ;ZIP")
include(CPack)
```

## 3. Sample BLang Programs

Let's create some example programs to showcase BLang's features.

### 3.1 Hello World

```
// examples/hello_world.bl
class Main {
    static func main() {
        Console.println("Hello, BLang!");
    }
}
```

### 3.2 Object-Oriented Features

```
// examples/shapes.bl
class Shape {
    func area() {
        return 0; // Base implementation
    }

    func perimeter() {
        return 0; // Base implementation
    }

    func describe() {
        Console.println("This is a shape with area: " + this.area() + " and perimeter: " + this.perimeter());
    }
}

class Circle extends Shape {
    var radius: Number;

    func constructor(radius: Number) {
        this.radius = radius;
    }

    func area() {
        return 3.14159 * this.radius * this.radius;
    }

    func perimeter() {
        return 2 * 3.14159 * this.radius;
    }
}

class Rectangle extends Shape {
    var width: Number;
    var height: Number;

    func constructor(width: Number, height: Number) {
        this.width = width;
        this.height = height;
    }

    func area() {
        return this.width * this.height;
    }

    func perimeter() {
        return 2 * (this.width + this.height);
    }
}

class Main {
    static func main() {
        var circle = new Circle(5);
        var rectangle = new Rectangle(4, 6);

        circle.describe();
        rectangle.describe();

        var shapes = new List();
        shapes.add(circle);
        shapes.add(rectangle);

        for (var i = 0; i < shapes.size(); i++) {
            var shape = shapes.get(i);
            Console.println("Shape " + i + " area: " + shape.area());
        }
    }
}
```

### 3.3 Exception Handling

```
// examples/exceptions.bl
class DivisionByZeroException {
    var message: String;

    func constructor(message: String) {
        this.message = message;
    }
}

class Calculator {
    func divide(a: Number, b: Number) {
        if (b == 0) {
            throw new DivisionByZeroException("Cannot divide by zero");
        }
        return a / b;
    }
}

class Main {
    static func main() {
        var calculator = new Calculator();

        try {
            var result = calculator.divide(10, 2);
            Console.println("Result: " + result);

            result = calculator.divide(10, 0);
            Console.println("This won't be reached");
        } catch (e: DivisionByZeroException) {
            Console.println("Caught exception: " + e.message);
        } finally {
            Console.println("Cleanup code runs always");
        }

        Console.println("Program continues after exception handling");
    }
}
```

## 4. Extending BLang

Let's look at ways to extend BLang with new features.

### 4.1 Adding New Language Features

To add new language features to BLang, you'll need to modify several components:

1. **Lexer**: Add new token types for new syntax
2. **Parser**: Update grammar rules to handle new syntax
3. **Type Checker**: Implement type checking for new features
4. **IR Generator**: Generate IR code for new constructs
5. **Virtual Machine**: Add support for executing new IR instructions

Let's walk through an example of adding a simple language feature: the `foreach` loop.

#### 4.1.1 Adding Foreach Loop

First, let's update the lexer to recognize the `foreach` keyword:

```cpp
// In Lexer.cpp
void Lexer::initializeKeywords() {
    // Existing keywords...
    keywords["foreach"] = TokenType::FOREACH;
    keywords["in"] = TokenType::IN;
}
```

Next, update the parser to handle the new syntax:

```cpp
// In Parser.h
std::shared_ptr<AST::Statement> parseForeachStatement();

// In Parser.cpp
std::shared_ptr<AST::Statement> Parser::parseStatement() {
    // Existing code...
    if (match(TokenType::FOREACH)) {
        return parseForeachStatement();
    }
    // Existing code...
}

std::shared_ptr<AST::Statement> Parser::parseForeachStatement() {
    // Parse foreach (var item in collection) { body }
    consume(TokenType::LEFT_PAREN, "Expect '(' after 'foreach'.");

    consume(TokenType::VAR, "Expect 'var' in foreach declaration.");
    Token name = consume(TokenType::IDENTIFIER, "Expect variable name.");

    consume(TokenType::IN, "Expect 'in' after variable name.");
    auto collection = parseExpression();

    consume(TokenType::RIGHT_PAREN, "Expect ')' after foreach condition.");

    auto body = parseStatement();

    return std::make_shared<AST::ForeachStatement>(name, collection, body);
}
```

Create a new AST node for foreach statements:

```cpp
// In AST.h
class ForeachStatement : public Statement {
private:
    Token variable;
    std::shared_ptr<Expression> collection;
    std::shared_ptr<Statement> body;

public:
    ForeachStatement(Token variable, std::shared_ptr<Expression> collection,
                    std::shared_ptr<Statement> body)
        : variable(variable), collection(collection), body(body) {}

    Token getVariable() const { return variable; }
    std::shared_ptr<Expression> getCollection() const { return collection; }
    std::shared_ptr<Statement> getBody() const { return body; }

    void accept(Visitor* visitor) override {
        visitor->visitForeachStatement(this);
    }
};
```

Update the type checker for the new statement:

```cpp
// In TypeChecker.cpp
void TypeChecker::visitForeachStatement(AST::ForeachStatement* stmt) {
    // Check collection expression
    stmt->getCollection()->accept(this);
    auto collectionType = currentExprType;

    // Verify the collection is iterable (has iterator() method)
    if (!isIterable(collectionType)) {
        errorReporter.error(stmt->getCollection()->getLocation(),
                           "Expression is not iterable");
        return;
    }

    // Enter a new scope for the loop variable
    symbolTable.enterScope();

    // Define the loop variable
    symbolTable.define(stmt->getVariable().lexeme, getElementType(collectionType));

    // Check the body
    stmt->getBody()->accept(this);

    // Exit the scope
    symbolTable.exitScope();
}

bool TypeChecker::isIterable(const Type& type) {
    return type.hasMethod("iterator");
}

Type TypeChecker::getElementType(const Type& collectionType) {
    // Get the type parameter from the collection
    // For simplicity, we'll just return the Object type
    return Type::object();
}
```

Add IR generation for foreach loops:

```cpp
// In IRGenerator.cpp
void IRGenerator::visitForeachStatement(AST::ForeachStatement* stmt) {
    // Generate IR for the collection expression
    stmt->getCollection()->accept(this);
    auto collection = lastValue;

    // Create basic blocks
    auto loopHeader = createBasicBlock("foreach.header");
    auto loopBody = createBasicBlock("foreach.body");
    auto loopExit = createBasicBlock("foreach.exit");

    // Get iterator from collection
    auto iterator = createTemporary();
    auto iteratorCall = std::make_shared<IR::CallMethod>(
        collection,
        std::make_shared<IR::Value>("iterator"),
        std::make_shared<IR::Value>("0")
    );
    addInstruction(iteratorCall);
    addInstruction(std::make_shared<IR::Assign>(iterator, iteratorCall));

    // Branch to loop header
    addInstruction(std::make_shared<IR::Jump>(loopHeader->getName()));

    // Loop header - check if there are more elements
    setCurrentBlock(loopHeader);
    auto hasNext = createTemporary();
    auto hasNextCall = std::make_shared<IR::CallMethod>(
        iterator,
        std::make_shared<IR::Value>("hasNext"),
        std::make_shared<IR::Value>("0")
    );
    addInstruction(hasNextCall);
    addInstruction(std::make_shared<IR::Assign>(hasNext, hasNextCall));

    // Branch based on hasNext
    addInstruction(std::make_shared<IR::Branch>(
        hasNext,
        loopBody->getName(),
        loopExit->getName()
    ));

    // Loop body
    setCurrentBlock(loopBody);

    // Get next element
    auto element = createTemporary();
    auto nextCall = std::make_shared<IR::CallMethod>(
        iterator,
        std::make_shared<IR::Value>("next"),
        std::make_shared<IR::Value>("0")
    );
    addInstruction(nextCall);
    addInstruction(std::make_shared<IR::Assign>(element, nextCall));

    // Store element in loop variable
    auto varName = stmt->getVariable().lexeme;
    addInstruction(std::make_shared<IR::DeclareVar>(
        std::make_shared<IR::Value>(varName)
    ));
    addInstruction(std::make_shared<IR::Assign>(
        std::make_shared<IR::Value>(varName),
        element
    ));

    // Generate IR for loop body
    stmt->getBody()->accept(this);

    // Jump back to header
    addInstruction(std::make_shared<IR::Jump>(loopHeader->getName()));

    // Loop exit
    setCurrentBlock(loopExit);
}
```

Finally, add VM support for iterator operations:

```cpp
// Add appropriate executeXXX methods to VirtualMachine.cpp
// for the new IR instructions, if necessary
```

### 4.2 Adding Standard Library Extensions

To extend BLang's standard library:

1. Create new classes with the desired functionality
2. Register them in the `StandardLibrary::initialize` method

For example, let's add a JSON library:

```cpp
// In StandardLibrary/JsonClass.h
class JsonClass {
public:
    static std::shared_ptr<ClassInstance> create(
        std::shared_ptr<ClassInstance> objectClass) {

        auto jsonClass = std::make_shared<ClassInstance>("JSON", objectClass);

        // Add static methods
        addParseMethod(jsonClass);
        addStringifyMethod(jsonClass);

        return jsonClass;
    }

private:
    static void addParseMethod(std::shared_ptr<ClassInstance> jsonClass) {
        auto parseImpl = std::make_shared<IR::Function>("JSON.parse");
        parseImpl->addParameter("text");

        // Implementation...

        auto parseMethod = std::make_shared<MethodInstance>(
            "parse",
            parseImpl,
            jsonClass,
            std::vector<std::string>{"text"},
            false
        );

        parseMethod->setStatic(true);
        jsonClass->addMethod("parse", parseMethod);
    }

    static void addStringifyMethod(std::shared_ptr<ClassInstance> jsonClass) {
        // Implementation...
    }
};

// Then in StandardLibrary.cpp:
void StandardLibrary::initialize(VirtualMachine& vm) {
    // Existing code...

    // Add JSON library
    auto jsonClass = JsonClass::create(objectClass);
    vm.registerClass(jsonClass);
}
```

## 5. Future Directions

Now that you've built a complete object-oriented programming language, here are some directions for further development:

### 5.1 Performance Optimization

- Implement a Just-In-Time (JIT) compiler for performance-critical code
- Add optimization passes to the IR generator
- Improve garbage collection efficiency

### 5.2 Language Enhancements

- Add support for generics
- Implement interfaces or traits
- Add closures and anonymous functions
- Support for asynchronous programming

### 5.3 Developer Tools

- Create a debugger for BLang
- Build a language server protocol (LSP) implementation for IDE integration
- Develop a package manager for BLang libraries

## 6. Exercises and Assignments

1. **Add a New Feature**: Implement the `switch` statement in BLang.

2. **Extend Standard Library**: Implement a JSON or XML library for BLang.

3. **Create a Package Manager**: Build a simple package manager for distributing BLang libraries.

4. **Implement a Compiler Frontend**: Create a compiler frontend that translates BLang to another target language (JavaScript, C++, etc.).

5. **Build a Sample Application**: Develop a non-trivial application in BLang (e.g., a web server, game, or utility).

## 7. Conclusion

Congratulations! You've successfully built a complete object-oriented programming language from scratch. Throughout this course, you've learned about:

- Lexical analysis and parsing
- Type systems and semantic analysis
- Object-oriented language design
- Intermediate representations
- Virtual machines and execution
- Memory management and garbage collection
- Standard library design

Building a programming language is a challenging but rewarding experience that deepens your understanding of programming language theory and implementation. The skills you've gained are applicable to many areas of software development, from building domain-specific languages to optimizing compilers and interpreters.

We hope this course has given you the knowledge and confidence to explore language design further and even create your own languages for specific domains or purposes.

Happy coding with BLang!

## 8. References and Further Reading

- Aho, A. V., Lam, M. S., Sethi, R., & Ullman, J. D. (2006). Compilers: Principles, Techniques, and Tools (2nd Edition).
- Appel, A. W. (2004). Modern Compiler Implementation in C.
- Pierce, B. C. (2002). Types and Programming Languages.
- Krishnamurthi, S. (2012). Programming Languages: Application and Interpretation.
- Scott, M. L. (2015). Programming Language Pragmatics (4th Edition).
