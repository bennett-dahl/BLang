# Lesson 8: Runtime and Memory Management

## Learning Objectives

- Implement a garbage collector for automatic memory management
- Design the runtime class hierarchy and inheritance system
- Support dynamic method dispatch for polymorphism
- Implement exception handling mechanisms

## 1. Memory Management in Object-Oriented Languages

Managing memory efficiently is crucial for any programming language. In an object-oriented language like BLang, objects are created and destroyed dynamically, which introduces several memory management challenges.

### 1.1 Memory Management Challenges

Object-oriented languages face several specific memory management challenges:

- **Dynamic Object Creation**: Objects are created at runtime with varying lifetimes
- **Object References**: Objects can reference other objects, creating complex object graphs
- **Inheritance**: Subclass instances contain all fields from their parent classes
- **Polymorphism**: The same reference can refer to different object types at runtime

Manual memory management (like in C/C++) is error-prone and can lead to:

- Memory leaks (forgetting to free memory)
- Dangling pointers (accessing memory after it's freed)
- Double free errors (freeing memory more than once)

For BLang, we'll implement automatic memory management through garbage collection.

### 1.2 Garbage Collection Approaches

There are several approaches to garbage collection:

1. **Reference Counting**: Maintain a count of references to each object

   - **Pros**: Simple to implement, immediate collection
   - **Cons**: Cannot handle cyclic references, overhead on reference updates

2. **Mark and Sweep**: Trace reachable objects from root references

   - **Pros**: Handles cycles, collects all garbage at once
   - **Cons**: Pauses program execution, can be inefficient for large heaps

3. **Generational**: Divide objects by age and collect younger objects more frequently

   - **Pros**: Efficient for programs that create many short-lived objects
   - **Cons**: More complex to implement

4. **Incremental/Concurrent**: Spread collection work over time or run alongside the program
   - **Pros**: Reduces pause times
   - **Cons**: Very complex to implement correctly

For BLang, we'll implement a simple mark-and-sweep collector first, as it's a good balance between effectiveness and implementation complexity.

## 2. Mark and Sweep Garbage Collection

Let's implement a basic mark-and-sweep garbage collector for BLang. This will involve three phases:

1. **Mark**: Trace through all reachable objects starting from root references
2. **Sweep**: Free memory for any objects that weren't marked
3. **Compact**: (Optional) Rearrange objects to reduce fragmentation

### 2.1 Tracking Object Allocations

First, we need to modify our object system to track all allocations. Let's create a memory manager class:

```cpp
#ifndef BLANG_MEMORY_MANAGER_H
#define BLANG_MEMORY_MANAGER_H

#include "RuntimeObject.h"
#include <unordered_set>
#include <vector>
#include <memory>

namespace BLang {

class MemoryManager {
private:
    // Set of all allocated objects
    std::unordered_set<ObjectInstance*> allocatedObjects;

    // Root references (global variables, etc.)
    std::vector<std::shared_ptr<ObjectInstance>*> rootReferences;

    // GC statistics
    size_t totalAllocated = 0;
    size_t totalCollected = 0;
    size_t collectionCount = 0;

    // Mark all objects reachable from roots
    void markReachableObjects();

    // Helper for marking objects recursively
    void markObject(ObjectInstance* obj);

    // Sweep unmarked objects
    void sweepUnmarkedObjects();

public:
    MemoryManager() = default;
    ~MemoryManager();

    // Allocate a new object
    template<typename T, typename... Args>
    std::shared_ptr<T> allocateObject(Args&&... args) {
        auto obj = std::make_shared<T>(std::forward<Args>(args)...);
        allocatedObjects.insert(obj.get());
        totalAllocated += sizeof(T);

        // Check if garbage collection is needed
        if (shouldCollectGarbage()) {
            collectGarbage();
        }

        return obj;
    }

    // Register a root reference
    void addRootReference(std::shared_ptr<ObjectInstance>* root) {
        rootReferences.push_back(root);
    }

    // Remove a root reference
    void removeRootReference(std::shared_ptr<ObjectInstance>* root) {
        auto it = std::find(rootReferences.begin(), rootReferences.end(), root);
        if (it != rootReferences.end()) {
            rootReferences.erase(it);
        }
    }

    // Check if garbage collection should be triggered
    bool shouldCollectGarbage() const {
        // Simple heuristic: collect if we have more than 1000 objects
        // or more than 1 MB of allocated memory
        return allocatedObjects.size() > 1000 || totalAllocated > 1024 * 1024;
    }

    // Perform garbage collection
    void collectGarbage();

    // Get statistics
    size_t getTotalAllocated() const { return totalAllocated; }
    size_t getTotalCollected() const { return totalCollected; }
    size_t getCollectionCount() const { return collectionCount; }
    size_t getObjectCount() const { return allocatedObjects.size(); }
};

} // namespace BLang

#endif // BLANG_MEMORY_MANAGER_H
```

### 2.2 Implementing the Mark and Sweep Algorithm

Now, let's update our object class to include a "marked" flag for garbage collection:

```cpp
class ObjectInstance {
private:
    std::shared_ptr<ClassInstance> classType;
    std::unordered_map<std::string, RuntimeValue> fields;
    bool marked = false;  // For garbage collection

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

    // Garbage collection methods
    bool isMarked() const { return marked; }
    void mark() { marked = true; }
    void unmark() { marked = false; }

    // Get all references to other objects
    std::vector<ObjectInstance*> getReferences() const {
        std::vector<ObjectInstance*> refs;
        for (const auto& [name, value] : fields) {
            if (value.isObject()) {
                refs.push_back(value.getObject().get());
            }
        }
        return refs;
    }
};
```

Next, let's implement the garbage collection methods in our MemoryManager:

```cpp
void MemoryManager::markReachableObjects() {
    // First, unmark all objects
    for (auto obj : allocatedObjects) {
        obj->unmark();
    }

    // Mark objects reachable from root references
    for (auto rootRef : rootReferences) {
        if (*rootRef) {
            markObject(rootRef->get());
        }
    }
}

void MemoryManager::markObject(ObjectInstance* obj) {
    if (!obj || obj->isMarked()) {
        return;  // Already marked or null
    }

    // Mark this object
    obj->mark();

    // Recursively mark all referenced objects
    for (auto ref : obj->getReferences()) {
        markObject(ref);
    }
}

void MemoryManager::sweepUnmarkedObjects() {
    auto it = allocatedObjects.begin();
    size_t freedCount = 0;
    size_t freedSize = 0;

    while (it != allocatedObjects.end()) {
        ObjectInstance* obj = *it;

        if (!obj->isMarked()) {
            // Object is not reachable, remove it
            size_t objSize = sizeof(*obj);  // Simplified size calculation
            freedSize += objSize;
            freedCount++;

            // Remove from the allocated set
            it = allocatedObjects.erase(it);

            // Note: We don't need to explicitly delete the object
            // because it's managed by shared_ptr, which will be
            // destroyed when all references are gone
        } else {
            // Object is still reachable
            ++it;
        }
    }

    // Update statistics
    totalCollected += freedSize;
    collectionCount++;
    totalAllocated -= freedSize;
}

void MemoryManager::collectGarbage() {
    // Perform mark and sweep
    markReachableObjects();
    sweepUnmarkedObjects();
}

MemoryManager::~MemoryManager() {
    // Clean up all remaining objects
    allocatedObjects.clear();
    rootReferences.clear();
}
```

### 2.3 Integration with the Virtual Machine

Now we need to integrate our garbage collector with the virtual machine. We'll modify the VirtualMachine class to use the MemoryManager for object allocation:

```cpp
class VirtualMachine {
private:
    std::shared_ptr<IR::Module> module;
    std::stack<RuntimeValue> operandStack;
    std::vector<StackFrame> callStack;
    std::unordered_map<std::string, std::shared_ptr<ClassInstance>> classes;

    // Add a memory manager
    MemoryManager memoryManager;

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

    // Specific instruction handlers
    void executeNew(std::shared_ptr<IR::Instruction> inst) {
        // Get the class name from the instruction
        std::string className = "SomeClass";  // Simplified

        // Look up the class
        auto classIt = classes.find(className);
        if (classIt == classes.end()) {
            throw std::runtime_error("Class not found: " + className);
        }

        // Create a new instance using the memory manager
        auto instance = memoryManager.allocateObject<ObjectInstance>(classIt->second);

        // Push the new instance onto the stack
        push(RuntimeValue(instance));
    }

    // Other execution methods...

public:
    VirtualMachine(std::shared_ptr<IR::Module> module)
        : module(module), currentFrame(nullptr) {}

    // Initialize the VM
    void initialize() {
        // Register globals as root references
        for (auto& [name, global] : globals) {
            memoryManager.addRootReference(&global);
        }
    }

    // Execute a function
    RuntimeValue executeFunction(const std::string& functionName,
                              const std::vector<RuntimeValue>& args);

    // Run the VM
    void run();

    // Get memory statistics
    MemoryStats getMemoryStats() const {
        return {
            memoryManager.getTotalAllocated(),
            memoryManager.getTotalCollected(),
            memoryManager.getCollectionCount(),
            memoryManager.getObjectCount()
        };
    }
};
```

## 3. Runtime Class Hierarchy

In an object-oriented language, the class hierarchy must be represented at runtime to support inheritance and polymorphism. Let's implement the necessary structures for BLang's runtime class system.

### 3.1 Class Representation

First, we need a way to represent classes at runtime:

```cpp
#ifndef BLANG_CLASS_INSTANCE_H
#define BLANG_CLASS_INSTANCE_H

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace BLang {

class MethodInstance;
class ClassInstance;

// Forward declaration
class RuntimeValue;

// Represents a runtime class
class ClassInstance {
private:
    std::string name;
    std::shared_ptr<ClassInstance> superClass;
    std::unordered_map<std::string, std::shared_ptr<MethodInstance>> methods;
    std::vector<std::string> fieldNames;

public:
    ClassInstance(const std::string& name, std::shared_ptr<ClassInstance> superClass = nullptr)
        : name(name), superClass(superClass) {}

    // Get class name
    const std::string& getName() const { return name; }

    // Get superclass (null if this is the root class)
    std::shared_ptr<ClassInstance> getSuperClass() const { return superClass; }

    // Check if this class is a subclass of another class
    bool isSubclassOf(const std::shared_ptr<ClassInstance>& other) const {
        if (other == nullptr) {
            return false;
        }
        if (this == other.get()) {
            return true;  // A class is considered a subclass of itself
        }
        if (superClass == nullptr) {
            return false;
        }
        return superClass->isSubclassOf(other);
    }

    // Add a method to this class
    void addMethod(const std::string& name, std::shared_ptr<MethodInstance> method) {
        methods[name] = method;
    }

    // Look up a method by name (including inherited methods)
    std::shared_ptr<MethodInstance> lookupMethod(const std::string& name) const {
        // First try to find the method in this class
        auto it = methods.find(name);
        if (it != methods.end()) {
            return it->second;
        }

        // If not found and we have a superclass, check there
        if (superClass) {
            return superClass->lookupMethod(name);
        }

        // Method not found
        return nullptr;
    }

    // Register a field for this class
    void addField(const std::string& name) {
        fieldNames.push_back(name);
    }

    // Get all field names (including inherited fields)
    std::vector<std::string> getAllFieldNames() const {
        std::vector<std::string> allFields;

        // First, add fields from superclass
        if (superClass) {
            allFields = superClass->getAllFieldNames();
        }

        // Then add our own fields
        allFields.insert(allFields.end(), fieldNames.begin(), fieldNames.end());

        return allFields;
    }
};

} // namespace BLang

#endif // BLANG_CLASS_INSTANCE_H
```

### 3.2 Method Representation

Next, we need to represent methods:

```cpp
#ifndef BLANG_METHOD_INSTANCE_H
#define BLANG_METHOD_INSTANCE_H

#include <string>
#include <memory>
#include <vector>
#include "IR/Function.h"

namespace BLang {

class ClassInstance;

// Represents a method in a class
class MethodInstance {
private:
    std::string name;
    std::shared_ptr<IR::Function> implementation;
    std::weak_ptr<ClassInstance> declaringClass;
    std::vector<std::string> parameterNames;
    bool isVirtual;  // Whether the method can be overridden

public:
    MethodInstance(const std::string& name,
                  std::shared_ptr<IR::Function> implementation,
                  std::shared_ptr<ClassInstance> declaringClass,
                  const std::vector<std::string>& parameterNames,
                  bool isVirtual = true)
        : name(name),
          implementation(implementation),
          declaringClass(declaringClass),
          parameterNames(parameterNames),
          isVirtual(isVirtual) {}

    // Get method name
    const std::string& getName() const { return name; }

    // Get the implementation
    std::shared_ptr<IR::Function> getImplementation() const { return implementation; }

    // Get declaring class
    std::shared_ptr<ClassInstance> getDeclaringClass() const {
        return declaringClass.lock();
    }

    // Get parameter names
    const std::vector<std::string>& getParameterNames() const { return parameterNames; }

    // Check if this method is virtual (can be overridden)
    bool isVirtualMethod() const { return isVirtual; }
};

} // namespace BLang

#endif // BLANG_METHOD_INSTANCE_H
```

### 3.3 Class Initialization

When loading a BLang program, we need to initialize all classes and their relationships:

```cpp
void VirtualMachine::initializeClasses() {
    // First, create all class objects
    for (const auto& classDecl : module->getClasses()) {
        std::shared_ptr<ClassInstance> classInst =
            std::make_shared<ClassInstance>(classDecl->getName());
        classes[classDecl->getName()] = classInst;
    }

    // Then, set up inheritance relationships and methods
    for (const auto& classDecl : module->getClasses()) {
        auto classInst = classes[classDecl->getName()];

        // Set up inheritance
        if (!classDecl->getSuperClassName().empty()) {
            auto superClass = classes[classDecl->getSuperClassName()];
            if (!superClass) {
                throw std::runtime_error("Super class not found: " +
                                         classDecl->getSuperClassName());
            }
            classInst = std::make_shared<ClassInstance>(classDecl->getName(), superClass);
            classes[classDecl->getName()] = classInst;
        }

        // Add fields
        for (const auto& field : classDecl->getFields()) {
            classInst->addField(field->getName());
        }

        // Add methods
        for (const auto& method : classDecl->getMethods()) {
            auto methodImpl = module->getFunction(classDecl->getName() + "." +
                                                method->getName());
            if (!methodImpl) {
                throw std::runtime_error("Method implementation not found: " +
                                         method->getName());
            }

            auto methodInst = std::make_shared<MethodInstance>(
                method->getName(),
                methodImpl,
                classInst,
                method->getParameterNames(),
                method->isVirtual()
            );

            classInst->addMethod(method->getName(), methodInst);
        }
    }
}
```

## 4. Dynamic Method Dispatch

One of the key features of object-oriented programming is polymorphism, which allows a single interface to represent different types. In BLang, we'll implement dynamic method dispatch to support this feature.

### 4.1 Method Invocation

Let's implement the method call instruction in our virtual machine:

```cpp
void VirtualMachine::executeMethodCall(std::shared_ptr<IR::Instruction> inst) {
    // Get the method name from the instruction
    std::string methodName = inst->getOperand(0)->getName();

    // Get the number of arguments
    int argCount = std::stoi(inst->getOperand(1)->getName());

    // Pop arguments from the stack
    std::vector<RuntimeValue> args;
    for (int i = 0; i < argCount; i++) {
        args.insert(args.begin(), pop());
    }

    // Pop the receiver object (this)
    RuntimeValue receiver = pop();

    if (!receiver.isObject()) {
        throw std::runtime_error("Cannot call method on non-object value");
    }

    auto object = receiver.getObject();
    auto classType = object->getClass();

    // Look up the method in the object's class
    auto method = classType->lookupMethod(methodName);
    if (!method) {
        throw std::runtime_error("Method not found: " + methodName);
    }

    // Create a new stack frame for the method call
    auto methodImpl = method->getImplementation();
    StackFrame frame(methodImpl);

    // Set the "this" parameter
    frame.setLocal("this", receiver);

    // Set method parameters
    const auto& paramNames = method->getParameterNames();
    for (size_t i = 0; i < std::min(paramNames.size(), args.size()); i++) {
        frame.setLocal(paramNames[i], args[i]);
    }

    // Push the frame onto the call stack
    callStack.push_back(frame);
    currentFrame = &callStack.back();
}
```

### 4.2 Virtual Method Table

For more efficient method dispatch, we can implement a virtual method table (vtable) optimization. This is similar to how C++ and other object-oriented languages implement virtual methods:

```cpp
class ClassInstance {
private:
    // Other members as before

    // Virtual method table (vtable)
    std::unordered_map<std::string, std::shared_ptr<MethodInstance>> vtable;

public:
    // Constructor now builds the vtable
    ClassInstance(const std::string& name, std::shared_ptr<ClassInstance> superClass = nullptr)
        : name(name), superClass(superClass) {
        // Initialize the vtable
        buildVTable();
    }

    // Build the virtual method table
    void buildVTable() {
        // First, inherit methods from superclass
        if (superClass) {
            vtable = superClass->getVTable();
        }

        // Then override with our own methods
        for (const auto& [name, method] : methods) {
            if (method->isVirtualMethod()) {
                vtable[name] = method;
            }
        }
    }

    // Get the entire vtable
    const std::unordered_map<std::string, std::shared_ptr<MethodInstance>>&
    getVTable() const {
        return vtable;
    }

    // More efficient method lookup using vtable
    std::shared_ptr<MethodInstance> lookupMethod(const std::string& name) const {
        // Look in the vtable for virtual methods
        auto it = vtable.find(name);
        if (it != vtable.end()) {
            return it->second;
        }

        // Look for non-virtual methods in this class only
        auto methodIt = methods.find(name);
        if (methodIt != methods.end() && !methodIt->second->isVirtualMethod()) {
            return methodIt->second;
        }

        // Method not found
        return nullptr;
    }

    // Other methods as before...
};
```

## 5. Exception Handling

Exception handling is an important feature in modern programming languages. Let's implement a simple exception mechanism for BLang.

### 5.1 Exception Representation

First, let's define how exceptions are represented:

```cpp
class Exception {
private:
    std::string type;
    std::string message;
    std::shared_ptr<ObjectInstance> data;

public:
    Exception(const std::string& type, const std::string& message,
             std::shared_ptr<ObjectInstance> data = nullptr)
        : type(type), message(message), data(data) {}

    const std::string& getType() const { return type; }
    const std::string& getMessage() const { return message; }
    std::shared_ptr<ObjectInstance> getData() const { return data; }
};
```

### 5.2 Try-Catch-Finally Mechanism

Now, let's add support for try-catch-finally blocks in our virtual machine:

```cpp
// Add these to the VirtualMachine class
private:
    // Exception handling
    struct ExceptionHandler {
        size_t tryStart;        // Start of try block
        size_t tryEnd;          // End of try block
        size_t catchStart;      // Start of catch block
        size_t finallyStart;    // Start of finally block
        std::string exceptionVar; // Name of exception variable in catch
    };

    std::vector<ExceptionHandler> exceptionHandlers;
    std::shared_ptr<Exception> currentException;

    // Find a handler for the current position
    ExceptionHandler* findHandler(size_t position) {
        for (auto& handler : exceptionHandlers) {
            if (position >= handler.tryStart && position < handler.tryEnd) {
                return &handler;
            }
        }
        return nullptr;
    }

public:
    // Throw an exception
    void throwException(const std::string& type, const std::string& message,
                       std::shared_ptr<ObjectInstance> data = nullptr) {
        currentException = std::make_shared<Exception>(type, message, data);

        // Unwind the stack to find a handler
        while (!callStack.empty()) {
            auto& frame = callStack.back();
            auto handler = findHandler(frame.getInstructionPointer());

            if (handler) {
                // Handler found, jump to catch block
                frame.setInstructionPointer(handler.catchStart);
                frame.setLocal(handler.exceptionVar,
                              RuntimeValue(currentException->getData()));
                return;
            }

            // No handler in this frame, check if there's a finally block
            handler = findHandler(frame.getInstructionPointer() - 1);
            if (handler && handler->finallyStart != 0) {
                // Execute finally block before popping the frame
                frame.setInstructionPointer(handler->finallyStart);
                executeUntilReturn();
            }

            // Pop the frame
            callStack.pop_back();

            if (!callStack.empty()) {
                currentFrame = &callStack.back();
            } else {
                currentFrame = nullptr;
                // No handler found, uncaught exception
                std::cerr << "Uncaught exception: " << currentException->getType()
                         << ": " << currentException->getMessage() << std::endl;
                break;
            }
        }
    }

    // Execute instructions until a return is encountered (for finally blocks)
    void executeUntilReturn() {
        while (currentFrame && !currentFrame->isReturning()) {
            auto instruction = currentFrame->getCurrentInstruction();
            executeInstruction(instruction);

            if (!currentFrame->isReturning()) {
                currentFrame->incrementInstructionPointer();
            }
        }
    }
```

### 5.3 IR Instructions for Exception Handling

We also need to add new IR instructions to support try-catch-finally blocks:

```cpp
// Add these to the IR::Instruction class hierarchy
class TryBegin : public Instruction {
public:
    TryBegin(std::shared_ptr<BasicBlock> catchBlock,
            std::shared_ptr<BasicBlock> finallyBlock,
            const std::string& exceptionVar)
        : Instruction(InstructionType::TryBegin) {
        addOperand(std::make_shared<Value>(catchBlock->getName()));
        addOperand(std::make_shared<Value>(finallyBlock ?
                                         finallyBlock->getName() : "null"));
        addOperand(std::make_shared<Value>(exceptionVar));
    }
};

class TryEnd : public Instruction {
public:
    TryEnd() : Instruction(InstructionType::TryEnd) {}
};

class Throw : public Instruction {
public:
    Throw(std::shared_ptr<Value> exceptionType,
         std::shared_ptr<Value> message,
         std::shared_ptr<Value> data = nullptr)
        : Instruction(InstructionType::Throw) {
        addOperand(exceptionType);
        addOperand(message);
        if (data) {
            addOperand(data);
        }
    }
};
```

### 5.4 Virtual Machine Support for Exception Instructions

Finally, let's implement the handlers for these new instructions:

```cpp
void VirtualMachine::executeTryBegin(std::shared_ptr<IR::Instruction> inst) {
    std::string catchBlockName = inst->getOperand(0)->getName();
    std::string finallyBlockName = inst->getOperand(1)->getName();
    std::string exceptionVar = inst->getOperand(2)->getName();

    // Find the basic blocks
    auto function = currentFrame->getFunction();
    auto catchBlock = function->getBasicBlock(catchBlockName);
    std::shared_ptr<IR::BasicBlock> finallyBlock = nullptr;
    if (finallyBlockName != "null") {
        finallyBlock = function->getBasicBlock(finallyBlockName);
    }

    // Register the exception handler
    ExceptionHandler handler;
    handler.tryStart = currentFrame->getInstructionPointer();
    handler.catchStart = catchBlock->getInstructionIndex();
    handler.finallyStart = finallyBlock ?
                          finallyBlock->getInstructionIndex() : 0;
    handler.exceptionVar = exceptionVar;

    // We'll set tryEnd when we encounter the TryEnd instruction
    exceptionHandlers.push_back(handler);
}

void VirtualMachine::executeTryEnd(std::shared_ptr<IR::Instruction> inst) {
    // Mark the end of the try block
    if (!exceptionHandlers.empty()) {
        exceptionHandlers.back().tryEnd = currentFrame->getInstructionPointer();
    }
}

void VirtualMachine::executeThrow(std::shared_ptr<IR::Instruction> inst) {
    std::string type = inst->getOperand(0)->getName();
    std::string message = inst->getOperand(1)->getName();

    std::shared_ptr<ObjectInstance> data = nullptr;
    if (inst->getOperandCount() > 2) {
        RuntimeValue dataValue = evaluate(inst->getOperand(2));
        if (dataValue.isObject()) {
            data = dataValue.getObject();
        }
    }

    throwException(type, message, data);
}
```

## 6. Exercises and Assignments

Now that you've implemented the core runtime system for BLang, try these exercises to deepen your understanding:

1. **Extend the Garbage Collector**: Implement a generational garbage collector that optimizes collection for short-lived objects.

2. **Optimize Method Dispatch**: Implement method inlining for frequently called methods.

3. **Add Custom Exception Types**: Extend the exception system to support different exception types with inheritance relationships.

4. **Implement Resource Management**: Add support for deterministic resource cleanup (similar to "using" in C# or "try-with-resources" in Java).

5. **Add Reflection Support**: Implement a basic reflection API that allows BLang programs to examine class structure at runtime.

## 7. Conclusion

In this lesson, we've built the runtime foundation for BLang, including:

- A garbage collector for automatic memory management
- A class hierarchy system to support inheritance
- Dynamic method dispatch for polymorphism
- An exception handling mechanism for error management

These components form the backbone of any modern object-oriented language. In the next lesson, we'll focus on building a standard library for BLang to provide essential functionality to users of our language.
