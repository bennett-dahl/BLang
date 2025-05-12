# Lesson 6: Intermediate Representation

## Learning Objectives

- Design an intermediate representation (IR) for BLang
- Transform the AST into IR code
- Implement simple optimizations
- Prepare for code generation or interpretation

## 1. Understanding Intermediate Representations

An Intermediate Representation (IR) bridges the gap between high-level source code and low-level execution. It simplifies complex language constructs and enables optimizations.

### 1.1 Why Use an IR?

- Simplifies code analysis and transformation
- Enables language-independent optimizations
- Creates a separation between front-end and back-end concerns
- Supports multiple target platforms

### 1.2 Types of IRs

- **High-level IR**: Close to source language
- **Mid-level IR**: Simplified for optimizations
- **Low-level IR**: Close to target machine code

## 2. Designing BLang's IR

For BLang, we'll use a three-address code representation where instructions have the form:

```
result = operand1 op operand2
```

Our IR will organize instructions into basic blocks, basic blocks into functions, and functions into modules.

### 2.1 IR Instruction Set

Our IR will include the following instruction types:

1. **Arithmetic Operations**

   - `ADD`: Addition (`result = operand1 + operand2`)
   - `SUB`: Subtraction (`result = operand1 - operand2`)
   - `MUL`: Multiplication (`result = operand1 * operand2`)
   - `DIV`: Division (`result = operand1 / operand2`)
   - `MOD`: Modulo (`result = operand1 % operand2`)
   - `NEG`: Negation (`result = -operand1`)

2. **Comparison Operations**

   - `EQ`: Equal (`result = operand1 == operand2`)
   - `NE`: Not Equal (`result = operand1 != operand2`)
   - `LT`: Less Than (`result = operand1 < operand2`)
   - `LE`: Less Than or Equal (`result = operand1 <= operand2`)
   - `GT`: Greater Than (`result = operand1 > operand2`)
   - `GE`: Greater Than or Equal (`result = operand1 >= operand2`)

3. **Logical Operations**

   - `AND`: Logical AND (`result = operand1 && operand2`)
   - `OR`: Logical OR (`result = operand1 || operand2`)
   - `NOT`: Logical NOT (`result = !operand1`)

4. **Control Flow**

   - `JUMP`: Unconditional jump to a label
   - `JUMPIF`: Conditional jump if operand is true
   - `JUMPIFNOT`: Conditional jump if operand is false
   - `LABEL`: Define a jump target

5. **Memory Operations**

   - `ALLOC`: Allocate memory for a variable
   - `LOAD`: Load a value from memory
   - `STORE`: Store a value to memory
   - `GETFIELD`: Get a field from an object
   - `SETFIELD`: Set a field on an object

6. **Function Operations**

   - `CALL`: Call a function with arguments
   - `RETURN`: Return from a function with a value
   - `PARAM`: Define a parameter for a function call

7. **Object Operations**
   - `NEW`: Create a new object instance
   - `INSTANCEOF`: Check if an object is an instance of a class

### 2.2 IR Structure

The IR will be structured hierarchically:

1. **Instructions**: The atomic operations described above
2. **Basic Blocks**: Sequences of instructions with a single entry point and a single exit point
3. **Functions**: Collections of basic blocks representing methods
4. **Module**: The complete program containing all functions and global data

This structure allows for efficient control flow analysis and optimization.

### 2.3 Registers and Variables

Our IR will use symbolic registers to represent intermediate values. These are not actual hardware registers but abstract locations for values. For example:

```
%1 = ADD %0, 5     // Add 5 to the value in register %0, store in %1
%2 = MUL %1, 2     // Multiply the value in %1 by 2, store in %2
```

Variables from the source code will be mapped to these registers during IR generation.

## 3. Implementing the IR Classes

Now, let's implement the core classes for our IR system. We'll define these in the `include` directory and implement them in the `src` directory.

### 3.1 IR Value and Type Classes

First, let's create `include/IR/IRValue.h` to define the base classes for values in our IR:

```cpp
#ifndef BLANG_IR_VALUE_H
#define BLANG_IR_VALUE_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace BLang {
namespace IR {

// Forward declarations
class IRType;
class Instruction;
class BasicBlock;
class Function;
class Module;

// Base class for all IR values
class IRValue {
public:
    enum class ValueKind {
        CONSTANT,
        REGISTER,
        INSTRUCTION,
        BASIC_BLOCK,
        FUNCTION,
        GLOBAL_VARIABLE
    };

private:
    ValueKind kind;
    std::string name;
    std::shared_ptr<IRType> type;

public:
    IRValue(ValueKind kind, const std::string& name, std::shared_ptr<IRType> type)
        : kind(kind), name(name), type(type) {}

    virtual ~IRValue() = default;

    ValueKind getKind() const { return kind; }
    const std::string& getName() const { return name; }
    std::shared_ptr<IRType> getType() const { return type; }

    // For debugging
    virtual void dump() const;
};

// IR constant value
class Constant : public IRValue {
private:
    // Union to store different types of constants
    union {
        int intValue;
        float floatValue;
        bool boolValue;
    };
    bool isString;
    std::string stringValue;

public:
    // Integer constant
    Constant(int value, std::shared_ptr<IRType> type)
        : IRValue(ValueKind::CONSTANT, "", type), intValue(value), isString(false) {}

    // Float constant
    Constant(float value, std::shared_ptr<IRType> type)
        : IRValue(ValueKind::CONSTANT, "", type), floatValue(value), isString(false) {}

    // Boolean constant
    Constant(bool value, std::shared_ptr<IRType> type)
        : IRValue(ValueKind::CONSTANT, "", type), boolValue(value), isString(false) {}

    // String constant
    Constant(const std::string& value, std::shared_ptr<IRType> type)
        : IRValue(ValueKind::CONSTANT, "", type), isString(true), stringValue(value) {}

    bool isInt() const;
    bool isFloat() const;
    bool isBool() const;
    bool isStringValue() const { return isString; }

    int getIntValue() const;
    float getFloatValue() const;
    bool getBoolValue() const;
    const std::string& getStringValue() const;

    void dump() const override;
};

// IR register (virtual register)
class Register : public IRValue {
private:
    int id;  // Unique identifier

public:
    Register(int id, std::shared_ptr<IRType> type)
        : IRValue(ValueKind::REGISTER, "%" + std::to_string(id), type), id(id) {}

    int getId() const { return id; }

    void dump() const override;
};

} // namespace IR
} // namespace BLang

#endif // BLANG_IR_VALUE_H
```

### 3.2 Implementing IR Module

After implementing the core IR value classes, let's create `include/IR/IRModule.h` to represent the complete module:

```cpp
#ifndef BLANG_IR_MODULE_H
#define BLANG_IR_MODULE_H

#include "Function.h"
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace BLang {
namespace IR {

// Module class (represents a complete program)
class Module {
private:
    std::string name;
    std::vector<std::shared_ptr<Function>> functions;
    std::unordered_map<std::string, std::shared_ptr<StructType>> structTypes;

    // Global variables would be added here

public:
    Module(const std::string& name) : name(name) {}

    const std::string& getName() const { return name; }

    // Function management
    void addFunction(std::shared_ptr<Function> function) {
        function->setParent(this);
        functions.push_back(function);
    }

    std::shared_ptr<Function> getFunction(const std::string& name) const {
        for (const auto& func : functions) {
            if (func->getName() == name) {
                return func;
            }
        }
        return nullptr;
    }

    const std::vector<std::shared_ptr<Function>>& getFunctions() const {
        return functions;
    }

    // Struct type management
    void addStructType(const std::string& name, std::shared_ptr<StructType> type) {
        structTypes[name] = type;
    }

    std::shared_ptr<StructType> getStructType(const std::string& name) const {
        auto it = structTypes.find(name);
        if (it != structTypes.end()) {
            return it->second;
        }
        return nullptr;
    }

    const std::unordered_map<std::string, std::shared_ptr<StructType>>& getStructTypes() const {
        return structTypes;
    }

    // For debugging
    void dump() const;
};

} // namespace IR
} // namespace BLang

#endif // BLANG_IR_MODULE_H
```

### 3.3 IR Builder Implementation

Now create a class to help build IR instructions. Create `include/IR/IRBuilder.h`:

```cpp
#ifndef BLANG_IR_BUILDER_H
#define BLANG_IR_BUILDER_H

#include "IRValue.h"
#include "IRType.h"
#include <memory>

namespace BLang {
namespace IR {

// Forward declarations
class Module;

// IRBuilder class
class IRBuilder {
private:
    std::shared_ptr<Module> module;

public:
    IRBuilder(std::shared_ptr<Module> module) : module(module) {}

    // Methods for creating IR instructions
    std::shared_ptr<Instruction> createAdd(std::shared_ptr<IRValue> left, std::shared_ptr<IRValue> right, std::shared_ptr<IRType> type);
    std::shared_ptr<Instruction> createSub(std::shared_ptr<IRValue> left, std::shared_ptr<IRValue> right, std::shared_ptr<IRType> type);
    std::shared_ptr<Instruction> createMul(std::shared_ptr<IRValue> left, std::shared_ptr<IRValue> right, std::shared_ptr<IRType> type);
    std::shared_ptr<Instruction> createDiv(std::shared_ptr<IRValue> left, std::shared_ptr<IRValue> right, std::shared_ptr<IRType> type);
    std::shared_ptr<Instruction> createMod(std::shared_ptr<IRValue> left, std::shared_ptr<IRValue> right, std::shared_ptr<IRType> type);
    std::shared_ptr<Instruction> createNeg(std::shared_ptr<IRValue> operand, std::shared_ptr<IRType> type);
    std::shared_ptr<Instruction> createEq(std::shared_ptr<IRValue> left, std::shared_ptr<IRValue> right, std::shared_ptr<IRType> type);
    std::shared_ptr<Instruction> createNe(std::shared_ptr<IRValue> left, std::shared_ptr<IRValue> right, std::shared_ptr<IRType> type);
    std::shared_ptr<Instruction> createLt(std::shared_ptr<IRValue> left, std::shared_ptr<IRValue> right, std::shared_ptr<IRType> type);
    std::shared_ptr<Instruction> createLe(std::shared_ptr<IRValue> left, std::shared_ptr<IRValue> right, std::shared_ptr<IRType> type);
    std::shared_ptr<Instruction> createGt(std::shared_ptr<IRValue> left, std::shared_ptr<IRValue> right, std::shared_ptr<IRType> type);
    std::shared_ptr<Instruction> createGe(std::shared_ptr<IRValue> left, std::shared_ptr<IRValue> right, std::shared_ptr<IRType> type);
    std::shared_ptr<Instruction> createAnd(std::shared_ptr<IRValue> left, std::shared_ptr<IRValue> right, std::shared_ptr<IRType> type);
    std::shared_ptr<Instruction> createOr(std::shared_ptr<IRValue> left, std::shared_ptr<IRValue> right, std::shared_ptr<IRType> type);
    std::shared_ptr<Instruction> createNot(std::shared_ptr<IRValue> operand, std::shared_ptr<IRType> type);
    std::shared_ptr<Instruction> createJump(std::shared_ptr<BasicBlock> target);
    std::shared_ptr<Instruction> createJumpiF(std::shared_ptr<IRValue> condition, std::shared_ptr<BasicBlock> trueTarget, std::shared_ptr<BasicBlock> falseTarget);
    std::shared_ptr<Instruction> createLabel(const std::string& label);
    std::shared_ptr<Instruction> createAlloc(std::shared_ptr<IRValue> size, std::shared_ptr<IRType> type);
    std::shared_ptr<Instruction> createLoad(std::shared_ptr<IRValue> address, std::shared_ptr<IRType> type);
    std::shared_ptr<Instruction> createStore(std::shared_ptr<IRValue> value, std::shared_ptr<IRValue> address);
    std::shared_ptr<Instruction> createGetField(std::shared_ptr<IRValue> object, const std::string& fieldName, std::shared_ptr<IRType> type);
    std::shared_ptr<Instruction> createSetField(std::shared_ptr<IRValue> object, const std::string& fieldName, std::shared_ptr<IRValue> value);
    std::shared_ptr<Instruction> createCall(std::shared_ptr<IRValue> function, const std::vector<std::shared_ptr<IRValue>>& arguments);
    std::shared_ptr<Instruction> createReturn(std::shared_ptr<IRValue> value);
    std::shared_ptr<Instruction> createParam(std::shared_ptr<IRValue> parameter);
    std::shared_ptr<Instruction> createNew(std::shared_ptr<IRType> type);
    std::shared_ptr<Instruction> createInstanceOf(std::shared_ptr<IRValue> object, std::shared_ptr<IRType> type);

    // Methods for creating IR values
    std::shared_ptr<IRValue> createConstant(int value, std::shared_ptr<IRType> type);
    std::shared_ptr<IRValue> createConstant(float value, std::shared_ptr<IRType> type);
    std::shared_ptr<IRValue> createConstant(bool value, std::shared_ptr<IRType> type);
    std::shared_ptr<IRValue> createConstant(const std::string& value, std::shared_ptr<IRType> type);
    std::shared_ptr<IRValue> createRegister(int id, std::shared_ptr<IRType> type);

    // Methods for creating IR types
    std::shared_ptr<IRType> createVoidType();
    std::shared_ptr<IRType> createIntType();
    std::shared_ptr<IRType> createFloatType();
    std::shared_ptr<IRType> createBoolType();
    std::shared_ptr<IRType> createStringType();
    std::shared_ptr<IRType> createPointerType(std::shared_ptr<IRType> elementType);
    std::shared_ptr<IRType> createArrayType(std::shared_ptr<IRType> elementType, int size);
    std::shared_ptr<IRType> createStructType(const std::vector<std::string>& fieldNames, const std::vector<std::shared_ptr<IRType>>& fieldTypes);
    std::shared_ptr<IRType> createFunctionType(std::shared_ptr<IRType> returnType, const std::vector<std::shared_ptr<IRType>>& paramTypes);

    // Methods for creating IR basic blocks
    std::shared_ptr<BasicBlock> createBasicBlock(const std::string& label);

    // Methods for creating IR functions
    std::shared_ptr<Function> createFunction(const std::string& name, std::shared_ptr<FunctionType> type, const std::vector<std::string>& paramNames);

    // Methods for creating IR modules
    std::shared_ptr<Module> createModule(const std::string& name);
};

} // namespace IR
} // namespace BLang

#endif // BLANG_IR_BUILDER_H
```

### 3.4 Development Workflow for IR Implementation

Follow these steps to implement the IR system:

1. **Create Header Files**:

   - Create the directory structure:

   ```bash
   mkdir -p include/IR
   ```

   - Create the header files in the include/IR directory
   - Define clear interfaces with documentation

2. **Implement Source Files**:

   - Create corresponding .cpp files in the src/IR directory:

   ```bash
   mkdir -p src/IR
   ```

   - Implement each class according to its header definition

3. **Update CMakeLists.txt**:

   ```cmake
   # Add the IR implementation files
   add_library(blang_lib
       # Previous components
       src/lexer.cpp
       src/parser.cpp
       src/ast.cpp
       src/semantics.cpp
       src/type.cpp
       # IR components
       src/IR/IRValue.cpp
       src/IR/IRType.cpp
       src/IR/IRInstruction.cpp
       src/IR/IRBasicBlock.cpp
       src/IR/IRFunction.cpp
       src/IR/IRModule.cpp
       src/IR/IRBuilder.cpp
   )
   ```

4. **Build and Test**:

   ```bash
   # From the project root
   cd build
   cmake ..
   cmake --build .
   ```

5. **Test the IR System**:
   Create tests in `test/ir_tests.cpp` to verify:

   - IR instruction creation
   - Basic block formation
   - Control flow between blocks
   - Full function generation

6. **Update test/CMakeLists.txt**:
   ```cmake
   add_executable(ir_tests ir_tests.cpp)
   target_link_libraries(ir_tests blang_lib)
   add_test(NAME IRTests COMMAND ir_tests)
   ```

## 4. AST to IR Translation

Now that we have defined the IR structure, we need to translate the AST produced by our parser into IR. This process is often called "lowering" as we're going from a high-level representation (AST) to a lower-level one (IR).

### 4.1 The IRGenerator Class

Let's create a class that will handle the translation from AST to IR:

```cpp
#ifndef BLANG_IR_GENERATOR_H
#define BLANG_IR_GENERATOR_H

#include "AST.h"
#include "IR/Module.h"
#include "SymbolTable.h"
#include "TypeSystem.h"
#include <memory>
#include <stack>
#include <unordered_map>

namespace BLang {

class IRGenerator {
private:
    std::shared_ptr<IR::Module> module;
    TypeSystem& typeSystem;
    SymbolTable& symbolTable;

    // Current function and basic block being generated
    std::shared_ptr<IR::Function> currentFunction;
    std::shared_ptr<IR::BasicBlock> currentBlock;

    // Next register ID
    int nextRegId = 0;

    // Map from AST variables to their IR registers
    std::unordered_map<std::string, std::shared_ptr<IR::Register>> variableMap;

    // Stack of blocks for control flow statements
    struct ControlFlowInfo {
        std::shared_ptr<IR::BasicBlock> continueBlock;
        std::shared_ptr<IR::BasicBlock> breakBlock;
    };
    std::stack<ControlFlowInfo> controlFlowStack;

    // Helper methods for generating IR
    std::shared_ptr<IR::Register> getNextRegister(std::shared_ptr<IR::IRType> type);
    std::shared_ptr<IR::BasicBlock> createBasicBlock(const std::string& name);
    void setCurrentBlock(std::shared_ptr<IR::BasicBlock> block);

    // Methods for converting AST types to IR types
    std::shared_ptr<IR::IRType> convertType(std::shared_ptr<Type> type);

    // Methods for generating IR from AST nodes
    std::shared_ptr<IR::IRValue> generateExpression(std::shared_ptr<Expression> expr);
    void generateStatement(std::shared_ptr<Statement> stmt);
    void generateDeclaration(std::shared_ptr<Declaration> decl);

    // Specific expression generators
    std::shared_ptr<IR::IRValue> generateLiteralExpr(std::shared_ptr<LiteralExpr> expr);
    std::shared_ptr<IR::IRValue> generateVariableExpr(std::shared_ptr<VariableExpr> expr);
    std::shared_ptr<IR::IRValue> generateBinaryExpr(std::shared_ptr<BinaryExpr> expr);
    std::shared_ptr<IR::IRValue> generateUnaryExpr(std::shared_ptr<UnaryExpr> expr);
    std::shared_ptr<IR::IRValue> generateCallExpr(std::shared_ptr<CallExpr> expr);
    std::shared_ptr<IR::IRValue> generateAssignExpr(std::shared_ptr<AssignExpr> expr);

    // Specific statement generators
    void generateExpressionStmt(std::shared_ptr<ExpressionStmt> stmt);
    void generateBlockStmt(std::shared_ptr<BlockStmt> stmt);
    void generateIfStmt(std::shared_ptr<IfStmt> stmt);
    void generateWhileStmt(std::shared_ptr<WhileStmt> stmt);
    void generateForStmt(std::shared_ptr<ForStmt> stmt);
    void generateReturnStmt(std::shared_ptr<ReturnStmt> stmt);
    void generateVarStmt(std::shared_ptr<VarStmt> stmt);

    // Declaration generators
    void generateClassDecl(std::shared_ptr<ClassDecl> decl);
    void generateMethodDecl(std::shared_ptr<MethodDecl> decl, std::shared_ptr<ClassSymbol> classSymbol);

public:
    IRGenerator(TypeSystem& typeSystem, SymbolTable& symbolTable)
        : typeSystem(typeSystem), symbolTable(symbolTable) {}

    std::shared_ptr<IR::Module> generate(std::shared_ptr<Program> program);
};

} // namespace BLang

#endif // BLANG_IR_GENERATOR_H
```

## 5. Simple Optimizations

After generating the IR, we can implement various optimizations to improve code efficiency. We won't go into all the details here, but some common optimizations include:

1. **Constant Folding**: Evaluating constant expressions at compile time
2. **Dead Code Elimination**: Removing unreachable or unused code
3. **Common Subexpression Elimination**: Avoiding redundant computations
4. **Loop Optimizations**: Such as loop unrolling and invariant code motion
5. **Tail Call Optimization**: Converting recursive calls to iterative loops

## Assignment

1. Complete the IRGenerator class by implementing:

   - Expression generation methods
   - Statement generation methods
   - Control flow handling for if/while/for statements
   - Method and class declarations

2. Implement a simple optimization pass:

   - Create a constant folding optimization
   - Add dead code elimination
   - Test the optimizations on sample BLang programs

3. Extend the IR system to support:
   - Exceptions and error handling
   - Generic types
   - Virtual method tables for inheritance

## Resources

- [Engineering a Compiler](https://www.amazon.com/Engineering-Compiler-Keith-Cooper/dp/012088478X) - Extensive coverage of intermediate representations and optimizations
- [LLVM Language Reference](https://llvm.org/docs/LangRef.html) - A real-world example of a powerful IR
- [Static Single Assignment Form](https://en.wikipedia.org/wiki/Static_single_assignment_form) - A property of IR that simplifies optimization

In the next lesson, we'll build on our IR to implement an interpreter for BLang.
