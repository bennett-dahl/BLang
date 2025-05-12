# Lesson 1: Foundations of Language Design

## Learning Objectives

- Understand the components of a programming language
- Define the goals and design principles of your language (BLang)
- Set up the development environment (C++, CMake, Git)
- Create a basic project structure

## 1. Understanding Programming Language Components

Programming languages are complex systems that consist of several critical components working together:

### 1.1 Lexical Analysis (Lexer/Scanner)

The lexer reads the source code as a stream of characters and groups them into meaningful tokens such as keywords, identifiers, literals, and operators. For example, the code `x = 5 + y;` would be tokenized as:

- Identifier: `x`
- Operator: `=`
- Number: `5`
- Operator: `+`
- Identifier: `y`
- Punctuation: `;`

### 1.2 Syntax Analysis (Parser)

The parser processes tokens to determine if the code follows the language's grammar rules. It typically produces an Abstract Syntax Tree (AST), which represents the hierarchical structure of the program. In our example, the AST might represent an assignment statement with the target `x` and the value being the result of adding `5` and `y`.

### 1.3 Semantic Analysis

This phase checks if the program makes logical sense. It verifies type compatibility, checks if variables are declared before use, and ensures operations are valid for their operands. For an object-oriented language, it also handles inheritance relationships and method resolution.

### 1.4 Intermediate Representation (IR)

Many language implementations translate the AST into an intermediate form that's easier to optimize or translate into machine code. This abstraction helps separate language semantics from target platform details.

### 1.5 Optimization

This optional phase improves code performance without changing its behavior. Optimizations can occur at various levels, from removing redundant operations to complex transformations.

### 1.6 Code Generation or Interpretation

The final phase either:

- Generates native machine code (compiler)
- Generates bytecode for a virtual machine (compiler + VM)
- Directly executes the program from the AST or IR (interpreter)

### 1.7 Runtime System

The runtime environment supports program execution, providing features like memory management, garbage collection, exception handling, and standard library functions.

## 2. Designing BLang: Our Object-Oriented Language

Let's define the goals and principles for our language, which we'll name "BLang":

### 2.1 Design Goals

- **Readability**: Clear, expressive syntax that's easy to understand
- **Safety**: Strong static typing to catch errors at compile time
- **Object-Oriented**: Full support for classes, inheritance, and encapsulation
- **Simplicity**: Minimize special cases and surprising behaviors
- **Modularity**: Support for organizing code into reusable modules

### 2.2 Key Features of BLang

- **Static Typing**: Variables and expressions have types determined at compile time
- **Classes and Objects**: Support for defining classes with methods and properties
- **Inheritance**: Single inheritance hierarchy with method overriding
- **Encapsulation**: Public, private, and protected access modifiers
- **Type Inference**: Automatically determine types when possible
- **Garbage Collection**: Automatic memory management
- **Helpful Error Messages**: Clear, concise feedback for programmers

### 2.3 BLang Syntax Preview

Here's a preview of what our language syntax will look like:

```blang
// Class definition
class Point {
    // Properties with access modifiers
    private int x;
    private int y;

    // Constructor
    public Point(int x, int y) {
        this.x = x;
        this.y = y;
    }

    // Methods
    public int getX() {
        return x;
    }

    public int getY() {
        return y;
    }

    public void move(int dx, int dy) {
        x += dx;
        y += dy;
    }

    // Method overriding will be supported
    public string toString() {
        return "Point(" + x + ", " + y + ")";
    }
}

// Inheritance
class ColorPoint extends Point {
    private string color;

    public ColorPoint(int x, int y, string color) {
        super(x, y); // Call parent constructor
        this.color = color;
    }

    public string getColor() {
        return color;
    }

    // Override method from parent class
    public string toString() {
        return "ColorPoint(" + getX() + ", " + getY() + ", " + color + ")";
    }
}

// Main function example
void main() {
    // Type inference with 'var'
    var p1 = new Point(10, 20);
    Point p2 = new Point(5, 15);

    p1.move(2, 3);

    // Output: Point(12, 23)
    print(p1.toString());

    // Polymorphism
    Point p3 = new ColorPoint(8, 16, "red");

    // Output: ColorPoint(8, 16, red)
    print(p3.toString());
}
```

## 3. Setting Up the Development Environment

To build our language, we'll need to set up a development environment with the necessary tools:

### 3.1 Installing Required Software

#### C++ Compiler

We'll use C++ for implementing our language. Make sure you have a modern C++ compiler:

- **Windows**: Install Visual Studio with C++ development tools or MinGW-w64
- **macOS**: Install Xcode Command Line Tools (`xcode-select --install`)
- **Linux**: Install GCC/G++ (`sudo apt install build-essential` for Ubuntu)

Verify your installation:

```bash
g++ --version  # or clang++ --version
```

#### CMake

CMake is a cross-platform build system generator that we'll use to manage our project:

- **Windows**: Download and install from [cmake.org](https://cmake.org/download/)
- **macOS**: Install via Homebrew: `brew install cmake`
- **Linux**: Install via package manager: `sudo apt install cmake`

Verify your installation:

```bash
cmake --version
```

#### Flex and Bison

Flex (lexer generator) and Bison (parser generator) will help us implement the lexer and parser:

- **Windows**: Install via MSYS2 or download binaries
- **macOS**: Install via Homebrew: `brew install flex bison`
- **Linux**: Install via package manager: `sudo apt install flex bison`

Verify your installation:

```bash
flex --version
bison --version
```

#### Git

We'll use Git for version control:

- **Windows**: Download and install from [git-scm.com](https://git-scm.com/download/win)
- **macOS**: Install via Homebrew: `brew install git` or Xcode
- **Linux**: Install via package manager: `sudo apt install git`

Verify your installation:

```bash
git --version
```

### 3.2 Project Structure

For this course, we're using the following directory structure:

- `/` - Root directory containing BLang implementation
  - `/src` - Source files (.cpp)
  - `/include` - Header files (.h)
  - `/test` - Test cases
  - `CMakeLists.txt` - Main build configuration

You'll implement the language components in these directories as you follow along with the lessons.

### 3.3 Basic C++ Development Workflow

Here's a basic workflow for implementing BLang components:

1. **Create Header Files**: Define your classes and interfaces in .h files in the `/include` directory
2. **Implement Source Files**: Write the implementation in .cpp files in the `/src` directory
3. **Update CMakeLists.txt**: Add new source files to the build configuration
4. **Build the Project**:
   ```bash
   # From the root directory:
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```
5. **Run Tests**:
   ```bash
   # From the build directory:
   ctest
   ```
6. **Run the BLang Interpreter**:
   ```bash
   # From the build directory:
   ./blang
   ```

Remember to follow consistent coding styles and document your code as you implement each component.

### 3.4 Initial Files

Our project already has the following files to get started:

- `CMakeLists.txt` - Main build configuration
- `src/main.cpp` - Entry point for the BLang interpreter
- `test/CMakeLists.txt` - Test configuration

As you progress through the lessons, you'll add more files to implement each language component.

## 4. Understanding the Challenge and Next Steps

Building a programming language is a complex but rewarding endeavor. In this lesson, we've set the foundation by:

1. Understanding the components of a language implementation
2. Designing the basic features and syntax of our language (BLang)
3. Setting up our development environment
4. Creating a starting project structure

In the next lesson, we'll dive deeper into lexical analysis, implementing the lexer that will transform source code into tokens. We'll use Flex to generate an efficient lexer based on regular expressions, and build the token representation that will be used throughout our language implementation.

## Assignment

1. Set up the development environment on your computer with all the required tools.
2. Create the basic project structure as outlined in this lesson.
3. Extend the Token and Lexer header files to add any additional token types or methods you think might be useful.
4. Think about what additional features you might want to add to the language and how they would impact the overall design.
5. Write a simple program using the BLang syntax shown in this lesson to get familiar with how the language will look.

## Resources

- [Compilers: Principles, Techniques, and Tools](https://www.amazon.com/Compilers-Principles-Techniques-Tools-2nd/dp/0321486811) (The Dragon Book)
- [Flex Documentation](https://westes.github.io/flex/manual/)
- [Bison Documentation](https://www.gnu.org/software/bison/manual/)
- [CMake Documentation](https://cmake.org/documentation/)
- [C++ Reference](https://en.cppreference.com/)
