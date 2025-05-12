# Lesson 7: Interpreter Implementation

## Learning Objectives

- Build a stack-based virtual machine
- Implement evaluation for BLang expressions and statements
- Handle method dispatch and dynamic binding
- Manage memory for objects and class instances

## 1. Introduction to Interpreters

In this lesson, we'll implement an interpreter for BLang. The interpreter will execute our intermediate representation (IR) directly without compiling it to machine code.

### 1.1 Types of Interpreters

There are several approaches to implementing an interpreter:

1. **AST Interpreters**: Directly traverse and execute the AST
2. **Bytecode Interpreters**: Execute a compact bytecode representation
3. **JIT Compilers**: Compile hot code paths to native code at runtime

For BLang, we'll implement a bytecode interpreter that executes our IR from Lesson 6.

### 1.2 Advantages of Interpreters

- Simpler to implement than a full compiler
- Easier to debug and add runtime checks
- Platform independence
- Support for dynamic features

## 2. Interpreter Architecture

Our interpreter will be based on a stack-based virtual machine model that executes the IR instructions sequentially.

### 2.1 Stack-Based Virtual Machine

In a stack-based VM, operations pop their operands from a stack and push results back onto the stack. This approach is simpler than a register-based VM and maps well to our IR.

The VM will have these main components:

1. **Operand Stack**: Holds intermediate values during computation
2. **Call Stack**: Tracks function calls and returns
3. **Heap**: Stores objects and other dynamically allocated data
4. **Instruction Pointer**: Points to the current instruction being executed

### 2.2 Runtime Data Structures

Let's define the core data structures for our interpreter. Create the file `include/Interpreter.h`:

```cpp
#ifndef BLANG_INTERPRETER_H
#define BLANG_INTERPRETER_H

#include "IR/Module.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <variant>
#include <stack>

namespace BLang {

// Forward declarations
class RuntimeValue;
class ObjectInstance;
class ClassInstance;

// Runtime value (represents a value at runtime)
class RuntimeValue {
public:
    enum class ValueKind {
        INT,
        FLOAT,
        BOOLEAN,
        STRING,
        OBJECT,
        ARRAY,
        NULL_VALUE
    };

private:
    ValueKind kind;

    // Union-like storage for different value types
    std::variant<int, float, bool, std::string, std::shared_ptr<ObjectInstance>> value;

public:
    // Constructors for different types
    RuntimeValue(int value) : kind(ValueKind::INT), value(value) {}
    RuntimeValue(float value) : kind(ValueKind::FLOAT), value(value) {}
    RuntimeValue(bool value) : kind(ValueKind::BOOLEAN), value(value) {}
    RuntimeValue(const std::string& value) : kind(ValueKind::STRING), value(value) {}
    RuntimeValue(std::shared_ptr<ObjectInstance> value) : kind(ValueKind::OBJECT), value(value) {}

    // Null value constructor
    RuntimeValue() : kind(ValueKind::NULL_VALUE) {}

    // Type checking
    bool isInt() const { return kind == ValueKind::INT; }
    bool isFloat() const { return kind == ValueKind::FLOAT; }
    bool isBoolean() const { return kind == ValueKind::BOOLEAN; }
    bool isString() const { return kind == ValueKind::STRING; }
    bool isObject() const { return kind == ValueKind::OBJECT; }
    bool isNull() const { return kind == ValueKind::NULL_VALUE; }

    // Value getters
    int getInt() const;
    float getFloat() const;
    bool getBoolean() const;
    const std::string& getString() const;
    std::shared_ptr<ObjectInstance> getObject() const;

    // Conversion utilities
    std::string toString() const;
};

// Stack frame (represents a function call)
class StackFrame {
private:
    std::shared_ptr<IR::Function> function;
    std::vector<RuntimeValue> locals;
    int instructionPointer;

public:
    StackFrame(std::shared_ptr<IR::Function> function)
        : function(function), instructionPointer(0) {
        // Initialize locals based on parameter count and local variables
        // This is simplified for brevity
        locals.resize(10);  // Placeholder value
    }

    std::shared_ptr<IR::Function> getFunction() const { return function; }

    RuntimeValue getLocal(int index) const {
        return locals[index];
    }

    void setLocal(int index, const RuntimeValue& value) {
        locals[index] = value;
    }

    int getIP() const { return instructionPointer; }
    void setIP(int ip) { instructionPointer = ip; }
    void incrementIP() { instructionPointer++; }
};

// Virtual Machine class
class VirtualMachine {
private:
    std::shared_ptr<IR::Module> module;
    std::stack<RuntimeValue> operandStack;
    std::vector<StackFrame> callStack;
    std::unordered_map<std::string, std::shared_ptr<ClassInstance>> classes;

    // Current execution state
    StackFrame* currentFrame;

    // Helper methods
    void push(const RuntimeValue& value) {
        operandStack.push(value);
    }

    RuntimeValue pop() {
        RuntimeValue value = operandStack.top();
        operandStack.pop();
        return value;
    }

    // Execute a single instruction
    void executeInstruction(std::shared_ptr<IR::Instruction> instruction);

    // Execute specific instruction types
    void executeBinaryOp(std::shared_ptr<IR::BinaryOpInst> inst);
    void executeJump(std::shared_ptr<IR::JumpInst> inst);
    void executeCondJump(std::shared_ptr<IR::CondJumpInst> inst);
    void executeCall(std::shared_ptr<IR::Instruction> inst);
    void executeReturn(std::shared_ptr<IR::ReturnInst> inst);
    void executeGetField(std::shared_ptr<IR::Instruction> inst);
    void executeSetField(std::shared_ptr<IR::Instruction> inst);
    void executeNew(std::shared_ptr<IR::Instruction> inst);

public:
    VirtualMachine(std::shared_ptr<IR::Module> module)
        : module(module), currentFrame(nullptr) {}

    // Initialize the VM
    void initialize();

    // Execute a function
    RuntimeValue executeFunction(const std::string& functionName, const std::vector<RuntimeValue>& args);

    // Run the VM
    void run();
};

} // namespace BLang

#endif // BLANG_INTERPRETER_H
```

### 2.3 Object Representation

Now, let's implement the class and object representation. Create the file `include/Object.h`:

```cpp
#ifndef BLANG_RUNTIME_OBJECT_H
#define BLANG_RUNTIME_OBJECT_H

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

namespace BLang {

// Forward declarations
class RuntimeValue;
class VirtualMethod;

// Object instance
class ObjectInstance {
private:
    std::shared_ptr<ClassInstance> classType;
    std::unordered_map<std::string, RuntimeValue> fields;

public:
    ObjectInstance(std::shared_ptr<ClassInstance> classType)
        : classType(classType) {}

    std::shared_ptr<ClassInstance> getClass() const { return classType; }

    void setField(const std::string& name, const RuntimeValue& value) {
        fields[name] = value;
    }

    RuntimeValue getField(const std::string& name) const {
        auto it = fields.find(name);
        if (it != fields.end()) {
            return it->second;
        }
        return RuntimeValue();  // Return null for nonexistent fields
    }
};

// Virtual method entry
class VirtualMethod {
private:
    std::string name;
    std::shared_ptr<IR::Function> implementation;

public:
    VirtualMethod(const std::string& name, std::shared_ptr<IR::Function> implementation)
        : name(name), implementation(implementation) {}

    const std::string& getName() const { return name; }
    std::shared_ptr<IR::Function> getImplementation() const { return implementation; }
};

// Class instance (runtime representation of a class)
class ClassInstance {
private:
    std::string name;
    std::shared_ptr<ClassInstance> superClass;
    std::vector<VirtualMethod> virtualMethods;
    std::vector<std::string> fieldNames;

public:
    ClassInstance(const std::string& name, std::shared_ptr<ClassInstance> superClass = nullptr)
        : name(name), superClass(superClass) {}

    const std::string& getName() const { return name; }
    std::shared_ptr<ClassInstance> getSuperClass() const { return superClass; }

    void addVirtualMethod(const VirtualMethod& method) {
        virtualMethods.push_back(method);
    }

    void addField(const std::string& name) {
        fieldNames.push_back(name);
    }

    std::shared_ptr<IR::Function> lookupMethod(const std::string& name) const {
        for (const auto& method : virtualMethods) {
            if (method.getName() == name) {
                return method.getImplementation();
            }
        }
        if (superClass) {
            return superClass->lookupMethod(name);
        }
        return nullptr;
    }

    bool hasField(const std::string& name) const {
        for (const auto& field : fieldNames) {
            if (field == name) {
                return true;
            }
        }
        if (superClass) {
            return superClass->hasField(name);
        }
        return false;
    }
};

} // namespace BLang

#endif // BLANG_RUNTIME_OBJECT_H
```

### 2.4 Development Workflow for the Interpreter

To implement the interpreter, follow these steps:

1. **Create Header Files**:

   - Add the interpreter headers to the `include` directory:

   ```bash
   # From the project root
   touch include/Interpreter.h
   touch include/Object.h
   ```

2. **Implement Source Files**:

   - Create the implementation files in the `src` directory:

   ```bash
   # From the project root
   touch src/Interpreter.cpp
   touch src/Object.cpp
   ```

3. **Update CMakeLists.txt**:

   ```cmake
   # Add the interpreter files
   add_library(blang_lib
       # Previous files
       src/lexer.cpp
       src/parser.cpp
       src/ast.cpp
       src/semantics.cpp
       src/type.cpp
       src/IR/IRValue.cpp
       # Interpreter files
       src/Interpreter.cpp
       src/Object.cpp
   )
   ```

4. **Build the Project**:

   ```bash
   # From the project root
   cd build
   cmake ..
   cmake --build .
   ```

5. **Update the Main File**:
   Update `src/main.cpp` to use the interpreter:

   ```cpp
   // In src/main.cpp
   #include "Interpreter.h"
   #include "IR/Module.h"
   #include <iostream>

   int main(int argc, char* argv[]) {
       // Process files from command line
       if (argc < 2) {
           std::cout << "Usage: blang <filename>" << std::endl;
           return 1;
       }

       // Create lexer, parser, semantics analyzer, and IR generator
       // ...

       // Create the interpreter
       BLang::VirtualMachine vm(irModule);
       vm.initialize();

       // Run the main function
       try {
           vm.executeFunction("main", {});
           std::cout << "Program executed successfully." << std::endl;
       } catch (const std::exception& e) {
           std::cerr << "Runtime error: " << e.what() << std::endl;
           return 1;
       }

       return 0;
   }
   ```

6. **Add Unit Tests**:
   Create tests in `test/interpreter_tests.cpp` to verify:

   - Basic expression evaluation
   - Control flow statements
   - Method calls
   - Object creation and field access

7. **Update test/CMakeLists.txt**:
   ```cmake
   add_executable(interpreter_tests interpreter_tests.cpp)
   target_link_libraries(interpreter_tests blang_lib)
   add_test(NAME InterpreterTests COMMAND interpreter_tests)
   ```

## 3. Implementing the Virtual Machine

Now, let's implement the core execution logic for our virtual machine.

### 3.1 Instruction Execution

The heart of our interpreter is the method to execute individual instructions:

```cpp
void VirtualMachine::executeInstruction(std::shared_ptr<IR::Instruction> instruction) {
    using OpCode = IR::Instruction::OpCode;

    switch (instruction->getOpCode()) {
        // Arithmetic operations
        case OpCode::ADD: {
            auto binOp = std::dynamic_pointer_cast<IR::BinaryOpInst>(instruction);
            executeBinaryOp(binOp);
            break;
        }
        case OpCode::SUB: {
            auto binOp = std::dynamic_pointer_cast<IR::BinaryOpInst>(instruction);
            executeBinaryOp(binOp);
            break;
        }
        case OpCode::MUL: {
            auto binOp = std::dynamic_pointer_cast<IR::BinaryOpInst>(instruction);
            executeBinaryOp(binOp);
            break;
        }
        case OpCode::DIV: {
            auto binOp = std::dynamic_pointer_cast<IR::BinaryOpInst>(instruction);
            executeBinaryOp(binOp);
            break;
        }

        // Control flow
        case OpCode::JUMP: {
            auto jumpInst = std::dynamic_pointer_cast<IR::JumpInst>(instruction);
            executeJump(jumpInst);
            break;
        }
        case OpCode::JUMPIF: {
            auto condJump = std::dynamic_pointer_cast<IR::CondJumpInst>(instruction);
            executeCondJump(condJump);
            break;
        }
        case OpCode::JUMPIFNOT: {
            auto condJump = std::dynamic_pointer_cast<IR::CondJumpInst>(instruction);
            executeCondJump(condJump);
            break;
        }

        // Function operations
        case OpCode::CALL: {
            executeCall(instruction);
            break;
        }
        case OpCode::RETURN: {
            auto returnInst = std::dynamic_pointer_cast<IR::ReturnInst>(instruction);
            executeReturn(returnInst);
            break;
        }

        // Object operations
        case OpCode::GETFIELD: {
            executeGetField(instruction);
            break;
        }
        case OpCode::SETFIELD: {
            executeSetField(instruction);
            break;
        }
        case OpCode::NEW: {
            executeNew(instruction);
            break;
        }

        // Other operations would be implemented here

        default:
            throw std::runtime_error("Unsupported instruction: " +
                                     IR::Instruction::opCodeToString(instruction->getOpCode()));
    }
}
```

### 3.2 Binary Operation Execution

Let's implement the execution of binary operations:

```cpp
void VirtualMachine::executeBinaryOp(std::shared_ptr<IR::BinaryOpInst> inst) {
    using OpCode = IR::Instruction::OpCode;

    // Pop operands in reverse order
    RuntimeValue right = pop();
    RuntimeValue left = pop();

    switch (inst->getOpCode()) {
        case OpCode::ADD:
            if (left.isInt() && right.isInt()) {
                push(RuntimeValue(left.getInt() + right.getInt()));
            } else if (left.isFloat() || right.isFloat()) {
                float leftVal = left.isFloat() ? left.getFloat() : static_cast<float>(left.getInt());
                float rightVal = right.isFloat() ? right.getFloat() : static_cast<float>(right.getInt());
                push(RuntimeValue(leftVal + rightVal));
            } else if (left.isString() || right.isString()) {
                // String concatenation
                push(RuntimeValue(left.toString() + right.toString()));
            } else {
                throw std::runtime_error("Type error in ADD operation");
            }
            break;

        case OpCode::SUB:
            if (left.isInt() && right.isInt()) {
                push(RuntimeValue(left.getInt() - right.getInt()));
            } else if (left.isFloat() || right.isFloat()) {
                float leftVal = left.isFloat() ? left.getFloat() : static_cast<float>(left.getInt());
                float rightVal = right.isFloat() ? right.getFloat() : static_cast<float>(right.getInt());
                push(RuntimeValue(leftVal - rightVal));
            } else {
                throw std::runtime_error("Type error in SUB operation");
            }
            break;

        case OpCode::MUL:
            if (left.isInt() && right.isInt()) {
                push(RuntimeValue(left.getInt() * right.getInt()));
            } else if (left.isFloat() || right.isFloat()) {
                float leftVal = left.isFloat() ? left.getFloat() : static_cast<float>(left.getInt());
                float rightVal = right.isFloat() ? right.getFloat() : static_cast<float>(right.getInt());
                push(RuntimeValue(leftVal * rightVal));
            } else {
                throw std::runtime_error("Type error in MUL operation");
            }
            break;

        case OpCode::DIV:
            if (right.isInt() && right.getInt() == 0 || right.isFloat() && right.getFloat() == 0.0f) {
                throw std::runtime_error("Division by zero");
            }

            if (left.isInt() && right.isInt()) {
                push(RuntimeValue(left.getInt() / right.getInt()));
            } else if (left.isFloat() || right.isFloat()) {
                float leftVal = left.isFloat() ? left.getFloat() : static_cast<float>(left.getInt());
                float rightVal = right.isFloat() ? right.getFloat() : static_cast<float>(right.getInt());
                push(RuntimeValue(leftVal / rightVal));
            } else {
                throw std::runtime_error("Type error in DIV operation");
            }
            break;

        // Additional operations would be implemented here

        default:
            throw std::runtime_error("Unsupported binary operation: " +
                                     IR::Instruction::opCodeToString(inst->getOpCode()));
    }
}
```

### 3.3 Object-Oriented Features

To support OOP features, we need to implement method dispatch and object creation:

```cpp
void VirtualMachine::executeNew(std::shared_ptr<IR::Instruction> inst) {
    // Get the class name from the instruction operands
    // This is simplified for brevity
    std::string className = "SomeClass"; // Would be extracted from instruction

    // Look up the class
    auto classIt = classes.find(className);
    if (classIt == classes.end()) {
        throw std::runtime_error("Class not found: " + className);
    }

    // Create a new instance of the class
    auto instance = std::make_shared<ObjectInstance>(classIt->second);

    // Initialize fields with default values
    // This is simplified for brevity

    // Push the new instance onto the stack
    push(RuntimeValue(instance));
}

void VirtualMachine::executeGetField(std::shared_ptr<IR::Instruction> inst) {
    // Get the field name from the instruction operands
    // This is simplified for brevity
    std::string fieldName = "someField"; // Would be extracted from instruction

    // Pop the object from the stack
    RuntimeValue objectValue = pop();
    if (!objectValue.isObject()) {
        throw std::runtime_error("Cannot access field on non-object value");
    }

    // Get the field value
    auto object = objectValue.getObject();
    RuntimeValue fieldValue = object->getField(fieldName);

    // Push the field value onto the stack
    push(fieldValue);
}

void VirtualMachine::executeSetField(std::shared_ptr<IR::Instruction> inst) {
    // Get the field name from the instruction operands
    // This is simplified for brevity
    std::string fieldName = "someField"; // Would be extracted from instruction

    // Pop the value and object from the stack
    RuntimeValue value = pop();
    RuntimeValue objectValue = pop();

    if (!objectValue.isObject()) {
        throw std::runtime_error("Cannot set field on non-object value");
    }

    // Set the field value
    auto object = objectValue.getObject();
    object->setField(fieldName, value);
}

void VirtualMachine::executeCall(std::shared_ptr<IR::Instruction> inst) {
    // Get method name and argument count
    // This is simplified for brevity
    std::string methodName = "someMethod"; // Would be extracted from instruction
    int argCount = 1; // Would be extracted from instruction

    // Pop arguments from the stack
    std::vector<RuntimeValue> args;
    for (int i = 0; i < argCount; i++) {
        args.push_back(pop());
    }
    std::reverse(args.begin(), args.end()); // Reverse since we popped in reverse order

    // Pop the object (if it's a method call)
    RuntimeValue objectValue = pop();

    if (objectValue.isObject()) {
        // Method call on an object
        auto object = objectValue.getObject();
        auto classType = object->getClass();

        // Look up the method in the class
        auto method = classType->lookupMethod(methodName);
        if (!method) {
            throw std::runtime_error("Method not found: " + methodName);
        }

        // Create a new stack frame
        callStack.push_back(StackFrame(method));
        currentFrame = &callStack.back();

        // Set up 'this' and parameters
        currentFrame->setLocal(0, objectValue); // 'this' is usually the first parameter
        for (size_t i = 0; i < args.size(); i++) {
            currentFrame->setLocal(i + 1, args[i]);
        }
    } else {
        // Static function call
        auto function = module->getFunction(methodName);
        if (!function) {
            throw std::runtime_error("Function not found: " + methodName);
        }

        // Create a new stack frame
        callStack.push_back(StackFrame(function));
        currentFrame = &callStack.back();

        // Set up parameters
        for (size_t i = 0; i < args.size(); i++) {
            currentFrame->setLocal(i, args[i]);
        }
    }
}
```

## 4. Memory Management

### 4.1 Memory Management Strategies

For BLang, we'll use a simple garbage collection approach to manage memory:

1. **Reference Counting**: Each object keeps track of how many references point to it
2. **Mark and Sweep**: Periodically identify and clean up unused objects
3. **Generational GC**: Handle short-lived and long-lived objects differently

For simplicity, we'll initially implement reference counting through the use of `std::shared_ptr` in C++.

### 4.2 Object Layout

The objects in our system will have the following memory layout:

1. **Header**: Contains type information and GC data
2. **Fields**: The actual data stored in the object
3. **Virtual Method Table (VMT)**: For polymorphic method dispatch

## 5. Next Steps

With our interpreter implementation, we now have a way to execute BLang programs. In the next lesson, we'll focus on advanced runtime features and memory management techniques.

## Assignment

1. Complete the virtual machine implementation by:

   - Implementing the remaining instruction execution methods
   - Adding support for arrays and exceptions
   - Creating a test suite for the interpreter

2. Enhance the object system with:

   - Method overriding and dynamic dispatch
   - Proper inheritance handling
   - Access control enforcement

3. Implement a simple garbage collector to manage memory.

## Resources

- [Crafting Interpreters](https://craftinginterpreters.com/) - A practical guide to implementing interpreters
- [Virtual Machine Design and Implementation](https://www.amazon.com/Virtual-Machine-Design-Implementation-Systems/dp/1598293250) - Detailed coverage of VM concepts
- [The Garbage Collection Handbook](https://www.amazon.com/Garbage-Collection-Handbook-Management-Algorithms/dp/1420082795) - Reference for memory management techniques
