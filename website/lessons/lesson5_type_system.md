# Lesson 5: Building the Type System

## Learning Objectives

- Design a robust type system for object-oriented programming
- Implement inheritance relationships between types
- Build a type inference mechanism
- Handle method resolution and polymorphism
- Implement subtyping relationships

## 1. Understanding Type Systems

A type system is a set of rules that assigns types to various constructs in a programming language. In statically-typed object-oriented languages like BLang, the type system serves several crucial functions:

1. **Detecting errors** at compile time before the program runs
2. **Documenting interfaces** between components
3. **Supporting polymorphism** through subtyping
4. **Enabling optimizations** by providing information to the compiler

### 1.1 Type System Design Principles

When designing a type system for an object-oriented language, consider these principles:

- **Safety**: Prevent operations that would cause runtime errors
- **Expressiveness**: Allow programmers to express their intent clearly
- **Predictability**: Make type relationships and conversions intuitive
- **Efficiency**: Support efficient implementation
- **Extensibility**: Allow for future language extensions

### 1.2 Static vs. Dynamic Typing

BLang uses static typing, where types are checked at compile time. This provides several advantages:

- Earlier error detection
- Better documentation
- Potential for more optimizations
- IDE support (autocompletion, refactoring)

## 2. Type System Implementation

Let's expand our type system implementation from Lesson 4. We'll implement a more comprehensive type hierarchy to support object-oriented features.

### 2.1 Type Hierarchy

Create the file `include/TypeSystem.h`:

```cpp
#ifndef BLANG_TYPE_SYSTEM_H
#define BLANG_TYPE_SYSTEM_H

#include "Type.h"
#include <unordered_map>
#include <memory>
#include <string>

namespace BLang {

class TypeSystem {
private:
    std::unordered_map<std::string, std::shared_ptr<Type>> types;

public:
    TypeSystem() {
        // Register primitive types
        registerType(Type::VOID_TYPE);
        registerType(Type::INT_TYPE);
        registerType(Type::FLOAT_TYPE);
        registerType(Type::BOOLEAN_TYPE);
        registerType(Type::STRING_TYPE);
    }

    void registerType(std::shared_ptr<Type> type) {
        types[type->getName()] = type;
    }

    std::shared_ptr<Type> lookupType(const std::string& name) const {
        auto it = types.find(name);
        if (it != types.end()) {
            return it->second;
        }
        return nullptr;
    }

    // Check if fromType is a subtype of toType
    bool isSubtype(std::shared_ptr<Type> fromType, std::shared_ptr<Type> toType) const {
        if (!fromType || !toType) {
            return false;
        }

        // Same type
        if (fromType == toType) {
            return true;
        }

        // Null is a subtype of any reference type
        if (fromType->getKind() == Type::Kind::NULL_TYPE &&
            toType->getKind() == Type::Kind::CLASS) {
            return true;
        }

        // Handle numeric conversions (int to float)
        if (fromType == Type::INT_TYPE && toType == Type::FLOAT_TYPE) {
            return true;
        }

        // Class subtyping
        if (fromType->getKind() == Type::Kind::CLASS && toType->getKind() == Type::Kind::CLASS) {
            auto fromClassType = std::dynamic_pointer_cast<ClassType>(fromType);
            auto toClassType = std::dynamic_pointer_cast<ClassType>(toType);

            return fromClassType->isSubtypeOf(toClassType);
        }

        return false;
    }

    // Find the common supertype (for conditionals, etc.)
    std::shared_ptr<Type> findCommonSupertype(std::shared_ptr<Type> type1,
                                             std::shared_ptr<Type> type2) const {
        if (!type1 || !type2) {
            return nullptr;
        }

        // Same type
        if (type1 == type2) {
            return type1;
        }

        // Handle numeric types
        if ((type1 == Type::INT_TYPE && type2 == Type::FLOAT_TYPE) ||
            (type1 == Type::FLOAT_TYPE && type2 == Type::INT_TYPE)) {
            return Type::FLOAT_TYPE;
        }

        // Handle class hierarchy
        if (type1->getKind() == Type::Kind::CLASS && type2->getKind() == Type::Kind::CLASS) {
            auto classType1 = std::dynamic_pointer_cast<ClassType>(type1);
            auto classType2 = std::dynamic_pointer_cast<ClassType>(type2);

            // Find lowest common ancestor in the class hierarchy
            return findCommonAncestor(classType1, classType2);
        }

        // If types are incompatible, return null
        return nullptr;
    }

private:
    std::shared_ptr<ClassType> findCommonAncestor(std::shared_ptr<ClassType> type1,
                                                std::shared_ptr<ClassType> type2) const {
        // If one is a subtype of the other, return the supertype
        if (type1->isSubtypeOf(type2)) {
            return type2;
        }
        if (type2->isSubtypeOf(type1)) {
            return type1;
        }

        // Otherwise, walk up type1's hierarchy until we find a common ancestor
        auto current = type1->getSuperClass();
        while (current) {
            if (type2->isSubtypeOf(current)) {
                return current;
            }
            current = current->getSuperClass();
        }

        // If no common ancestor, return Object (the root)
        return lookupObjectType();
    }

    std::shared_ptr<ClassType> lookupObjectType() const {
        auto objectType = std::dynamic_pointer_cast<ClassType>(lookupType("Object"));
        if (!objectType) {
            // This should not happen in a well-formed type system
            throw std::runtime_error("Object type not found in type system");
        }
        return objectType;
    }
};

} // namespace BLang

#endif // BLANG_TYPE_SYSTEM_H
```

### 2.2 Enhanced Type Classes

Let's update our `Type.h` file to add more sophisticated type handling:

```cpp
#ifndef BLANG_TYPE_H
#define BLANG_TYPE_H

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

namespace BLang {

// Forward declarations
class ClassSymbol;
class MethodSymbol;

// Base type class
class Type {
public:
    enum class Kind {
        VOID,
        INT,
        FLOAT,
        BOOLEAN,
        STRING,
        CLASS,
        ARRAY,
        NULL_TYPE  // For null literal
    };

private:
    Kind kind;
    std::string name;

public:
    Type(Kind kind, const std::string& name) : kind(kind), name(name) {}
    virtual ~Type() = default;

    Kind getKind() const { return kind; }
    const std::string& getName() const { return name; }

    virtual bool isAssignableFrom(std::shared_ptr<Type> other) const {
        // By default, types are only assignable from themselves
        return this == other.get();
    }

    static std::shared_ptr<Type> VOID_TYPE;
    static std::shared_ptr<Type> INT_TYPE;
    static std::shared_ptr<Type> FLOAT_TYPE;
    static std::shared_ptr<Type> BOOLEAN_TYPE;
    static std::shared_ptr<Type> STRING_TYPE;
    static std::shared_ptr<Type> NULL_TYPE;
};

// Class type
class ClassType : public Type {
private:
    std::shared_ptr<ClassType> superClass;
    std::unordered_map<std::string, std::shared_ptr<MethodSymbol>> methods;
    std::unordered_map<std::string, std::shared_ptr<Type>> fields;

public:
    ClassType(const std::string& name)
        : Type(Kind::CLASS, name), superClass(nullptr) {}

    void setSuperClass(std::shared_ptr<ClassType> super) {
        superClass = super;
    }

    std::shared_ptr<ClassType> getSuperClass() const {
        return superClass;
    }

    void addMethod(const std::string& name, std::shared_ptr<MethodSymbol> method) {
        methods[name] = method;
    }

    void addField(const std::string& name, std::shared_ptr<Type> fieldType) {
        fields[name] = fieldType;
    }

    std::shared_ptr<MethodSymbol> lookupMethod(const std::string& name) const {
        auto it = methods.find(name);
        if (it != methods.end()) {
            return it->second;
        }
        if (superClass) {
            return superClass->lookupMethod(name);
        }
        return nullptr;
    }

    std::shared_ptr<Type> lookupField(const std::string& name) const {
        auto it = fields.find(name);
        if (it != fields.end()) {
            return it->second;
        }
        if (superClass) {
            return superClass->lookupField(name);
        }
        return nullptr;
    }

    bool isSubtypeOf(std::shared_ptr<ClassType> other) const {
        if (this == other.get()) {
            return true;
        }
        if (!superClass) {
            return false;
        }
        return superClass->isSubtypeOf(other);
    }

    bool isAssignableFrom(std::shared_ptr<Type> other) const override {
        if (other->getKind() == Kind::NULL_TYPE) {
            // Null is assignable to any class type
            return true;
        }

        if (other->getKind() != Kind::CLASS) {
            return false;
        }

        auto otherClass = std::dynamic_pointer_cast<ClassType>(other);
        return otherClass->isSubtypeOf(std::shared_ptr<ClassType>(
            const_cast<ClassType*>(this), [](ClassType*){}));
    }
};

// Array type
class ArrayType : public Type {
private:
    std::shared_ptr<Type> elementType;

public:
    ArrayType(std::shared_ptr<Type> elementType)
        : Type(Kind::ARRAY, elementType->getName() + "[]"),
          elementType(elementType) {}

    std::shared_ptr<Type> getElementType() const {
        return elementType;
    }

    bool isAssignableFrom(std::shared_ptr<Type> other) const override {
        if (other->getKind() == Kind::NULL_TYPE) {
            // Null is assignable to any array type
            return true;
        }

        if (other->getKind() != Kind::ARRAY) {
            return false;
        }

        auto otherArray = std::dynamic_pointer_cast<ArrayType>(other);
        return elementType->isAssignableFrom(otherArray->getElementType());
    }
};

// Method type (for function types)
class MethodType : public Type {
private:
    std::shared_ptr<Type> returnType;
    std::vector<std::shared_ptr<Type>> parameterTypes;

public:
    MethodType(std::shared_ptr<Type> returnType,
               const std::vector<std::shared_ptr<Type>>& parameterTypes)
        : Type(Kind::CLASS, "Function"),
          returnType(returnType), parameterTypes(parameterTypes) {}

    std::shared_ptr<Type> getReturnType() const {
        return returnType;
    }

    const std::vector<std::shared_ptr<Type>>& getParameterTypes() const {
        return parameterTypes;
    }

    bool isCompatibleWith(const MethodType& other) const {
        // Check return type compatibility (covariant returns)
        if (!other.returnType->isAssignableFrom(returnType)) {
            return false;
        }

        // Check parameter compatibility (contravariant parameters)
        if (parameterTypes.size() != other.parameterTypes.size()) {
            return false;
        }

        for (size_t i = 0; i < parameterTypes.size(); i++) {
            if (!parameterTypes[i]->isAssignableFrom(other.parameterTypes[i])) {
                return false;
            }
        }

        return true;
    }
};

// Initialize primitive types
std::shared_ptr<Type> Type::VOID_TYPE = std::make_shared<Type>(Type::Kind::VOID, "void");
std::shared_ptr<Type> Type::INT_TYPE = std::make_shared<Type>(Type::Kind::INT, "int");
std::shared_ptr<Type> Type::FLOAT_TYPE = std::make_shared<Type>(Type::Kind::FLOAT, "float");
std::shared_ptr<Type> Type::BOOLEAN_TYPE = std::make_shared<Type>(Type::Kind::BOOLEAN, "boolean");
std::shared_ptr<Type> Type::STRING_TYPE = std::make_shared<Type>(Type::Kind::STRING, "string");
std::shared_ptr<Type> Type::NULL_TYPE = std::make_shared<Type>(Type::Kind::NULL_TYPE, "null");

} // namespace BLang

#endif // BLANG_TYPE_H
```

### 2.3 Implementing Type Operations

Now, let's implement the methods for working with types in `src/Type.cpp`:

```cpp
// ... existing code ...
```

### 2.4 Building and Testing the Type System

To ensure our type system is working correctly, add the following to your development workflow:

1. **Update CMakeLists.txt**: Add the type system files to your build:

```cmake
# In your main CMakeLists.txt
add_library(blang_lib
    # Previous files
    src/lexer.cpp
    src/parser.cpp
    src/ast.cpp
    src/semantics.cpp
    # Type system files
    src/type.cpp
    src/typesystem.cpp
)
```

2. **Build the Project**:

```bash
# From the project root directory
cd build
cmake ..
cmake --build .
```

3. **Create Test Cases**: Create tests to verify type relationships:

```cpp
// in test/type_tests.cpp
#include "Type.h"
#include "TypeSystem.h"
#include <cassert>
#include <iostream>

using namespace BLang;

int main() {
    TypeSystem typeSystem;

    // Test primitive types
    auto intType = Type::INT_TYPE;
    auto floatType = Type::FLOAT_TYPE;
    auto boolType = Type::BOOLEAN_TYPE;

    // Test subtyping relationships
    assert(typeSystem.isSubtype(intType, intType));  // Reflexivity
    assert(typeSystem.isSubtype(intType, floatType)); // int -> float conversion
    assert(!typeSystem.isSubtype(floatType, intType)); // float -> int not allowed
    assert(!typeSystem.isSubtype(intType, boolType)); // int -> bool not allowed

    std::cout << "Primitive type tests passed!" << std::endl;

    // Test class hierarchy
    // ... add class hierarchy tests here

    return 0;
}
```

4. **Update test/CMakeLists.txt**:

```cmake
add_executable(type_tests type_tests.cpp)
target_link_libraries(type_tests blang_lib)
add_test(NAME TypeTests COMMAND type_tests)
```

## 3. Class Types and Inheritance

In an object-oriented language, the subtyping relationship is crucial for polymorphism and code reuse. Let's implement the subtyping rules for BLang.

### 3.1 Subtyping Rules

The subtyping relation in BLang follows these principles:

1. **Reflexivity**: A type is a subtype of itself
2. **Transitivity**: If A is a subtype of B, and B is a subtype of C, then A is a subtype of C
3. **Class inheritance**: If class A extends class B, then A is a subtype of B
4. **Null type**: The null type is a subtype of any reference type (classes and arrays)
5. **Primitive types**: Limited subtyping (int can be converted to float)

### 3.2 Implementing Subtype Checking

We've already implemented the `isSubtype` method in our `TypeSystem` class. This method checks if one type is a subtype of another.

### 3.3 Subtyping and Method Overriding

In object-oriented languages, method overriding must respect the _Liskov Substitution Principle_:

```cpp
bool checkValidOverride(std::shared_ptr<MethodSymbol> baseMethod,
                      std::shared_ptr<MethodSymbol> overrideMethod,
                      const TypeSystem& typeSystem) {
    // Return type must be a subtype of the base method's return type (covariant return)
    if (!typeSystem.isSubtype(overrideMethod->getType(), baseMethod->getType())) {
        return false;
    }

    // Parameters must be compatible (contravariant)
    auto baseParams = baseMethod->getParameters();
    auto overrideParams = overrideMethod->getParameters();

    if (baseParams.size() != overrideParams.size()) {
        return false;
    }

    for (size_t i = 0; i < baseParams.size(); i++) {
        // Parameters should be contravariant, but in practice many languages
        // just require them to be the same type
        if (baseParams[i]->getType() != overrideParams[i]->getType()) {
            return false;
        }
    }

    return true;
}
```

## 4. Type Inference

Type inference allows the compiler to deduce types without explicit annotations, making the language more convenient to use.

### 4.1 Local Variable Type Inference

For local variables, we can infer the type from the initializer:

```cpp
std::shared_ptr<Type> inferVariableType(std::shared_ptr<Expression> initializer,
                                      const TypeSystem& typeSystem,
                                      const SymbolTable& symbolTable) {
    // If there's no initializer, we can't infer the type
    if (!initializer) {
        return nullptr;
    }

    // Use the expression type system to get the initializer's type
    std::shared_ptr<Type> initType = analyzeExpression(initializer, typeSystem, symbolTable);

    // Check for null initializer (requires explicit type)
    if (initType == Type::NULL_TYPE) {
        return nullptr;  // Can't infer from null without explicit type
    }

    return initType;
}
```

### 4.2 Method Return Type Inference

For methods, we can analyze the return statements to infer the return type:

```cpp
std::shared_ptr<Type> inferMethodReturnType(std::shared_ptr<BlockStmt> body,
                                          const TypeSystem& typeSystem,
                                          const SymbolTable& symbolTable) {
    // Find all return statements
    class ReturnVisitor {
    public:
        std::vector<std::shared_ptr<ReturnStmt>> returnStmts;

        void visit(std::shared_ptr<Statement> stmt) {
            if (auto returnStmt = std::dynamic_pointer_cast<ReturnStmt>(stmt)) {
                returnStmts.push_back(returnStmt);
            } else if (auto blockStmt = std::dynamic_pointer_cast<BlockStmt>(stmt)) {
                for (auto& s : blockStmt->getStatements()) {
                    visit(s);
                }
            } else if (auto ifStmt = std::dynamic_pointer_cast<IfStmt>(stmt)) {
                visit(ifStmt->getThenBranch());
                if (ifStmt->getElseBranch()) {
                    visit(ifStmt->getElseBranch());
                }
            } else if (auto whileStmt = std::dynamic_pointer_cast<WhileStmt>(stmt)) {
                visit(whileStmt->getBody());
            } else if (auto forStmt = std::dynamic_pointer_cast<ForStmt>(stmt)) {
                visit(forStmt->getBody());
            }
        }
    };

    ReturnVisitor visitor;
    visitor.visit(body);

    // If no return statements, infer void
    if (visitor.returnStmts.empty()) {
        return Type::VOID_TYPE;
    }

    // Analyze the type of each return value
    std::shared_ptr<Type> inferredType = nullptr;
    for (auto& returnStmt : visitor.returnStmts) {
        if (!returnStmt->getValue()) {
            // If any return statement has no value, the return type must be void
            return Type::VOID_TYPE;
        }

        auto valueType = analyzeExpression(returnStmt->getValue(), typeSystem, symbolTable);
        if (!inferredType) {
            inferredType = valueType;
        } else {
            // Find the common supertype of all return values
            inferredType = typeSystem.findCommonSupertype(inferredType, valueType);
            if (!inferredType) {
                // Incompatible return types
                return nullptr;
            }
        }
    }

    return inferredType;
}
```

## 5. Method Resolution and Polymorphism

Method resolution in an object-oriented language must handle both static and dynamic dispatch.

### 5.1 Static vs. Dynamic Method Dispatch

- **Static dispatch** is determined at compile time based on the declared type
- **Dynamic dispatch** is determined at runtime based on the actual object type

In BLang, instance methods use dynamic dispatch, while static methods use static dispatch.

### 5.2 Method Resolution Algorithm

The method resolution algorithm determines which method to call:

```cpp
std::shared_ptr<MethodSymbol> resolveMethod(std::shared_ptr<ClassType> classType,
                                          const std::string& methodName,
                                          const std::vector<std::shared_ptr<Expression>>& arguments,
                                          const TypeSystem& typeSystem,
                                          const SymbolTable& symbolTable) {
    // First, find all methods with the given name in the class hierarchy
    std::vector<std::shared_ptr<MethodSymbol>> candidates;

    std::shared_ptr<ClassType> currentClass = classType;
    while (currentClass) {
        auto method = currentClass->lookupMethod(methodName);
        if (method) {
            candidates.push_back(method);
        }
        currentClass = currentClass->getSuperClass();
    }

    // If no candidates, method doesn't exist
    if (candidates.empty()) {
        return nullptr;
    }

    // If we have arguments, find the best match based on argument types
    if (!arguments.empty()) {
        std::vector<std::shared_ptr<Type>> argTypes;
        for (auto& arg : arguments) {
            argTypes.push_back(analyzeExpression(arg, typeSystem, symbolTable));
        }

        // Find methods with matching parameter counts
        std::vector<std::shared_ptr<MethodSymbol>> matchingCandidates;
        for (auto& method : candidates) {
            if (method->getParameters().size() == argTypes.size()) {
                matchingCandidates.push_back(method);
            }
        }

        // If no matching candidates, method doesn't exist with right arity
        if (matchingCandidates.empty()) {
            return nullptr;
        }

        // Find the most specific method that can accept the arguments
        std::shared_ptr<MethodSymbol> bestMatch = nullptr;
        for (auto& method : matchingCandidates) {
            // Check if arguments are compatible with parameters
            bool isCompatible = true;
            for (size_t i = 0; i < argTypes.size(); i++) {
                auto paramType = method->getParameters()[i]->getType();
                if (!paramType->isAssignableFrom(argTypes[i])) {
                    isCompatible = false;
                    break;
                }
            }

            if (isCompatible) {
                if (!bestMatch) {
                    bestMatch = method;
                } else {
                    // If both methods are compatible, choose the more specific one
                    // This is a simplified version of method resolution
                    auto bestClass = std::dynamic_pointer_cast<ClassType>(
                        bestMatch->getParameters()[0]->getType());
                    auto currentClass = std::dynamic_pointer_cast<ClassType>(
                        method->getParameters()[0]->getType());

                    if (currentClass->isSubtypeOf(bestClass)) {
                        bestMatch = method;
                    }
                }
            }
        }

        return bestMatch;
    }

    // If no arguments, just return the first candidate (usually from the most derived class)
    return candidates[0];
}
```

## 6. Next Steps

With our type system implementation complete, we now have a foundation for robust type checking in our language. In the next lesson, we'll build on this to create an intermediate representation (IR) for our language, which will serve as the bridge between the AST and the interpreter or code generator.

## Assignment

1. Complete the type system implementation by:

   - Implementing the remaining methods for the `TypeSystem` class
   - Adding support for arrays and generic types
   - Creating test cases for the type system

2. Extend the semantic analyzer from Lesson 4 to use the new type system:

   - Update the type checking methods to use `isSubtype` instead of direct comparison
   - Implement method overloading resolution
   - Add support for type inference

3. Enhance the type system with:
   - Interfaces and multiple inheritance
   - Generic types (templates)
   - Union types

## Resources

- [Types and Programming Languages](https://www.amazon.com/Types-Programming-Languages-MIT-Press/dp/0262162091) by Benjamin C. Pierce
- [Advanced Topics in Types and Programming Languages](https://www.amazon.com/Advanced-Topics-Types-Programming-Languages/dp/0262162288)
- [Practical Foundations for Programming Languages](http://www.cs.cmu.edu/~rwh/pfpl.html) by Robert Harper
