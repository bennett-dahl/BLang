# Lesson 4: Semantic Analysis

## Learning Objectives

- Implement type checking and semantic validation
- Build a symbol table for variables, functions, and classes
- Handle scoping rules and variable resolution
- Validate object-oriented constructs (inheritance, method overriding)

## 1. Understanding Semantic Analysis

Semantic analysis is the phase that follows lexical and syntax analysis in a compiler or interpreter. While syntax analysis checks if the program follows the grammar rules, semantic analysis ensures that the program makes logical sense according to the language's typing rules and scoping rules.

The key responsibilities of semantic analysis include:

1. **Type checking**: Ensuring types are used correctly and compatible
2. **Symbol resolution**: Connecting variable/function uses with their declarations
3. **Scope handling**: Tracking which variables are visible in different parts of the program
4. **Validation of language-specific rules**: Such as inheritance, method overriding, and access control in object-oriented languages

## 2. Building a Symbol Table

A symbol table is a data structure that maps identifiers (variable names, function names, etc.) to their attributes (types, scopes, etc.). It's a crucial component for semantic analysis.

### 2.1 Symbol Table Structure

Let's define our symbol table structure:

```cpp
#ifndef BLANG_SYMBOL_TABLE_H
#define BLANG_SYMBOL_TABLE_H

#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include "AST.h"
#include "Type.h"

namespace BLang {

// Forward declarations
class Type;
class ClassType;

// Symbol kinds
enum class SymbolKind {
    VARIABLE,
    PARAMETER,
    METHOD,
    CLASS
};

// Symbol class (base class for all symbols)
class Symbol {
protected:
    std::string name;
    SymbolKind kind;
    std::shared_ptr<Type> type;

public:
    Symbol(const std::string& name, SymbolKind kind, std::shared_ptr<Type> type)
        : name(name), kind(kind), type(type) {}

    virtual ~Symbol() = default;

    const std::string& getName() const { return name; }
    SymbolKind getKind() const { return kind; }
    std::shared_ptr<Type> getType() const { return type; }
};

// Variable symbol
class VariableSymbol : public Symbol {
private:
    bool isField;
    AccessModifier access;

public:
    VariableSymbol(const std::string& name, std::shared_ptr<Type> type,
                   bool isField = false, AccessModifier access = AccessModifier::PUBLIC)
        : Symbol(name, SymbolKind::VARIABLE, type), isField(isField), access(access) {}

    bool getIsField() const { return isField; }
    AccessModifier getAccess() const { return access; }
};

// Parameter symbol
class ParameterSymbol : public Symbol {
public:
    ParameterSymbol(const std::string& name, std::shared_ptr<Type> type)
        : Symbol(name, SymbolKind::PARAMETER, type) {}
};

// Method symbol
class MethodSymbol : public Symbol {
private:
    std::vector<std::shared_ptr<ParameterSymbol>> parameters;
    AccessModifier access;
    bool isConstructor;

public:
    MethodSymbol(const std::string& name, std::shared_ptr<Type> returnType,
                 const std::vector<std::shared_ptr<ParameterSymbol>>& parameters,
                 AccessModifier access, bool isConstructor = false)
        : Symbol(name, SymbolKind::METHOD, returnType),
          parameters(parameters), access(access), isConstructor(isConstructor) {}

    const std::vector<std::shared_ptr<ParameterSymbol>>& getParameters() const { return parameters; }
    AccessModifier getAccess() const { return access; }
    bool getIsConstructor() const { return isConstructor; }
};

// Class symbol
class ClassSymbol : public Symbol {
private:
    std::shared_ptr<ClassSymbol> superClass;
    std::unordered_map<std::string, std::shared_ptr<VariableSymbol>> fields;
    std::unordered_map<std::string, std::shared_ptr<MethodSymbol>> methods;

public:
    ClassSymbol(const std::string& name, std::shared_ptr<ClassType> type,
                std::shared_ptr<ClassSymbol> superClass = nullptr)
        : Symbol(name, SymbolKind::CLASS, type), superClass(superClass) {}

    std::shared_ptr<ClassSymbol> getSuperClass() const { return superClass; }

    void addField(std::shared_ptr<VariableSymbol> field) {
        fields[field->getName()] = field;
    }

    void addMethod(std::shared_ptr<MethodSymbol> method) {
        methods[method->getName()] = method;
    }

    std::shared_ptr<VariableSymbol> getField(const std::string& name) const {
        auto it = fields.find(name);
        if (it != fields.end()) {
            return it->second;
        }
        if (superClass) {
            return superClass->getField(name);
        }
        return nullptr;
    }

    std::shared_ptr<MethodSymbol> getMethod(const std::string& name) const {
        auto it = methods.find(name);
        if (it != methods.end()) {
            return it->second;
        }
        if (superClass) {
            return superClass->getMethod(name);
        }
        return nullptr;
    }

    const std::unordered_map<std::string, std::shared_ptr<VariableSymbol>>& getFields() const {
        return fields;
    }

    const std::unordered_map<std::string, std::shared_ptr<MethodSymbol>>& getMethods() const {
        return methods;
    }
};

// Scope class (for tracking variable visibility)
class Scope {
private:
    Scope* parent;
    std::unordered_map<std::string, std::shared_ptr<Symbol>> symbols;

public:
    Scope(Scope* parent = nullptr) : parent(parent) {}

    void define(std::shared_ptr<Symbol> symbol) {
        symbols[symbol->getName()] = symbol;
    }

    std::shared_ptr<Symbol> resolve(const std::string& name) const {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            return it->second;
        }
        if (parent) {
            return parent->resolve(name);
        }
        return nullptr;
    }

    Scope* getParent() const { return parent; }
};

// Symbol table class
class SymbolTable {
private:
    Scope globalScope;
    Scope* currentScope;
    std::unordered_map<std::string, std::shared_ptr<ClassSymbol>> classes;

public:
    SymbolTable() : currentScope(&globalScope) {}

    void enterScope() {
        Scope* newScope = new Scope(currentScope);
        currentScope = newScope;
    }

    void exitScope() {
        if (currentScope->getParent()) {
            Scope* oldScope = currentScope;
            currentScope = currentScope->getParent();
            delete oldScope;
        }
    }

    void define(std::shared_ptr<Symbol> symbol) {
        currentScope->define(symbol);
    }

    std::shared_ptr<Symbol> resolve(const std::string& name) const {
        return currentScope->resolve(name);
    }

    void defineClass(std::shared_ptr<ClassSymbol> classSymbol) {
        classes[classSymbol->getName()] = classSymbol;
        define(classSymbol);
    }

    std::shared_ptr<ClassSymbol> getClass(const std::string& name) const {
        auto it = classes.find(name);
        if (it != classes.end()) {
            return it->second;
        }
        return nullptr;
    }
};

} // namespace BLang

#endif // BLANG_SYMBOL_TABLE_H
```

### 2.2 Type System

Let's also implement a basic type system:

```cpp
#ifndef BLANG_TYPE_H
#define BLANG_TYPE_H

#include <string>
#include <memory>
#include <vector>

namespace BLang {

// Forward declaration
class ClassSymbol;

// Base type class
class Type {
public:
    enum class Kind {
        VOID,
        INT,
        FLOAT,
        BOOLEAN,
        STRING,
        CLASS
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
        return this == other.get();
    }

    static std::shared_ptr<Type> VOID_TYPE;
    static std::shared_ptr<Type> INT_TYPE;
    static std::shared_ptr<Type> FLOAT_TYPE;
    static std::shared_ptr<Type> BOOLEAN_TYPE;
    static std::shared_ptr<Type> STRING_TYPE;
};

// Class type
class ClassType : public Type {
private:
    std::shared_ptr<ClassSymbol> classSymbol;

public:
    ClassType(const std::string& name)
        : Type(Kind::CLASS, name), classSymbol(nullptr) {}

    void setClassSymbol(std::shared_ptr<ClassSymbol> symbol) {
        classSymbol = symbol;
    }

    std::shared_ptr<ClassSymbol> getClassSymbol() const {
        return classSymbol;
    }

    bool isAssignableFrom(std::shared_ptr<Type> other) const override;
};

// Initialize primitive types
std::shared_ptr<Type> Type::VOID_TYPE = std::make_shared<Type>(Type::Kind::VOID, "void");
std::shared_ptr<Type> Type::INT_TYPE = std::make_shared<Type>(Type::Kind::INT, "int");
std::shared_ptr<Type> Type::FLOAT_TYPE = std::make_shared<Type>(Type::Kind::FLOAT, "float");
std::shared_ptr<Type> Type::BOOLEAN_TYPE = std::make_shared<Type>(Type::Kind::BOOLEAN, "boolean");
std::shared_ptr<Type> Type::STRING_TYPE = std::make_shared<Type>(Type::Kind::STRING, "string");

} // namespace BLang

#endif // BLANG_TYPE_H
```

## 3. Implementing Semantic Analysis

Now, let's create a semantic analyzer that uses the symbol table and type system to check for semantic errors.

### 3.1 Semantic Analyzer Interface

```cpp
#ifndef BLANG_SEMANTIC_ANALYZER_H
#define BLANG_SEMANTIC_ANALYZER_H

#include "AST.h"
#include "SymbolTable.h"
#include "Type.h"
#include <memory>
#include <vector>
#include <string>

namespace BLang {

class SemanticError {
private:
    std::string message;
    int line;
    int column;

public:
    SemanticError(const std::string& message, int line, int column)
        : message(message), line(line), column(column) {}

    const std::string& getMessage() const { return message; }
    int getLine() const { return line; }
    int getColumn() const { return column; }
};

class SemanticAnalyzer {
private:
    SymbolTable symbolTable;
    std::vector<SemanticError> errors;
    std::shared_ptr<ClassSymbol> currentClass;
    std::shared_ptr<MethodSymbol> currentMethod;

    // Helper methods for analysis
    void analyzeClass(std::shared_ptr<ClassDecl> classDecl);
    void analyzeMethod(std::shared_ptr<MethodDecl> methodDecl);
    void analyzeStatement(std::shared_ptr<Statement> stmt);
    std::shared_ptr<Type> analyzeExpression(std::shared_ptr<Expression> expr);

    // Type checking helpers
    bool checkAssignable(std::shared_ptr<Type> target, std::shared_ptr<Type> value,
                        int line, int column);
    std::shared_ptr<Type> getCommonType(std::shared_ptr<Type> type1, std::shared_ptr<Type> type2);

    // Symbol resolution helpers
    std::shared_ptr<VariableSymbol> resolveVariable(const std::string& name, int line, int column);
    std::shared_ptr<MethodSymbol> resolveMethod(std::shared_ptr<ClassSymbol> classSymbol,
                                              const std::string& name, int line, int column);

    // Error reporting
    void error(const std::string& message, int line, int column);

public:
    SemanticAnalyzer();

    // Analyze a program and return true if no errors
    bool analyze(std::shared_ptr<Program> program);

    // Get any errors found during analysis
    const std::vector<SemanticError>& getErrors() const { return errors; }
};

} // namespace BLang

#endif // BLANG_SEMANTIC_ANALYZER_H
```

### 3.2 Implementing Semantic Analyzer Methods

Let's implement some key methods of the semantic analyzer:

```cpp
#include "SemanticAnalyzer.h"
#include "DeclNodes.h"
#include "StmtNodes.h"
#include "ExprNodes.h"
#include <iostream>

namespace BLang {

SemanticAnalyzer::SemanticAnalyzer()
    : currentClass(nullptr), currentMethod(nullptr) {
}

bool SemanticAnalyzer::analyze(std::shared_ptr<Program> program) {
    errors.clear();

    // First pass: Register all classes
    for (const auto& decl : program->getDeclarations()) {
        if (auto classDecl = std::dynamic_pointer_cast<ClassDecl>(decl)) {
            auto className = classDecl->getName();
            auto classType = std::make_shared<ClassType>(className);
            auto classSymbol = std::make_shared<ClassSymbol>(className, classType);

            symbolTable.defineClass(classSymbol);
            classType->setClassSymbol(classSymbol);
        }
    }

    // Second pass: Establish inheritance relationships
    for (const auto& decl : program->getDeclarations()) {
        if (auto classDecl = std::dynamic_pointer_cast<ClassDecl>(decl)) {
            auto classSymbol = symbolTable.getClass(classDecl->getName());

            if (!classDecl->getSuperClass().empty()) {
                auto superClass = symbolTable.getClass(classDecl->getSuperClass());
                if (!superClass) {
                    error("Superclass '" + classDecl->getSuperClass() + "' not found", 0, 0);
                } else {
                    auto classType = std::dynamic_pointer_cast<ClassType>(classSymbol->getType());
                    classType->setClassSymbol(classSymbol);
                }
            }
        }
    }

    // Third pass: Analyze class bodies
    for (const auto& decl : program->getDeclarations()) {
        if (auto classDecl = std::dynamic_pointer_cast<ClassDecl>(decl)) {
            analyzeClass(classDecl);
        }
    }

    return errors.empty();
}

void SemanticAnalyzer::analyzeClass(std::shared_ptr<ClassDecl> classDecl) {
    auto className = classDecl->getName();
    currentClass = symbolTable.getClass(className);

    // Process fields
    for (const auto& field : classDecl->getFields()) {
        auto fieldName = field->getName();
        auto fieldTypeName = field->getTypeName();
        auto access = field->getAccess();

        // Resolve field type
        std::shared_ptr<Type> fieldType;
        if (fieldTypeName == "int") {
            fieldType = Type::INT_TYPE;
        } else if (fieldTypeName == "float") {
            fieldType = Type::FLOAT_TYPE;
        } else if (fieldTypeName == "boolean") {
            fieldType = Type::BOOLEAN_TYPE;
        } else if (fieldTypeName == "string") {
            fieldType = Type::STRING_TYPE;
        } else {
            auto classType = symbolTable.getClass(fieldTypeName);
            if (!classType) {
                error("Unknown type '" + fieldTypeName + "'", 0, 0);
                fieldType = Type::VOID_TYPE;
            } else {
                fieldType = classType->getType();
            }
        }

        auto fieldSymbol = std::make_shared<VariableSymbol>(fieldName, fieldType, true, access);
        currentClass->addField(fieldSymbol);

        // Check field initializer if present
        if (field->getInitializer()) {
            symbolTable.enterScope();
            auto initType = analyzeExpression(field->getInitializer());
            symbolTable.exitScope();

            checkAssignable(fieldType, initType, 0, 0);
        }
    }

    // Process methods
    for (const auto& method : classDecl->getMethods()) {
        analyzeMethod(method);
    }

    currentClass = nullptr;
}

void SemanticAnalyzer::analyzeMethod(std::shared_ptr<MethodDecl> methodDecl) {
    auto methodName = methodDecl->getName();
    auto returnTypeName = methodDecl->getReturnTypeName();
    auto isConstructor = methodDecl->getIsConstructor();

    // Resolve return type
    std::shared_ptr<Type> returnType;
    if (returnTypeName == "void") {
        returnType = Type::VOID_TYPE;
    } else if (returnTypeName == "int") {
        returnType = Type::INT_TYPE;
    } else if (returnTypeName == "float") {
        returnType = Type::FLOAT_TYPE;
    } else if (returnTypeName == "boolean") {
        returnType = Type::BOOLEAN_TYPE;
    } else if (returnTypeName == "string") {
        returnType = Type::STRING_TYPE;
    } else {
        auto classType = symbolTable.getClass(returnTypeName);
        if (!classType) {
            error("Unknown return type '" + returnTypeName + "'", 0, 0);
            returnType = Type::VOID_TYPE;
        } else {
            returnType = classType->getType();
        }
    }

    // Process parameters
    std::vector<std::shared_ptr<ParameterSymbol>> paramSymbols;
    for (const auto& param : methodDecl->getParameters()) {
        auto paramName = param.getName();
        auto paramTypeName = param.getTypeName();

        // Resolve parameter type
        std::shared_ptr<Type> paramType;
        if (paramTypeName == "int") {
            paramType = Type::INT_TYPE;
        } else if (paramTypeName == "float") {
            paramType = Type::FLOAT_TYPE;
        } else if (paramTypeName == "boolean") {
            paramType = Type::BOOLEAN_TYPE;
        } else if (paramTypeName == "string") {
            paramType = Type::STRING_TYPE;
        } else {
            auto classType = symbolTable.getClass(paramTypeName);
            if (!classType) {
                error("Unknown parameter type '" + paramTypeName + "'", 0, 0);
                paramType = Type::VOID_TYPE;
            } else {
                paramType = classType->getType();
            }
        }

        auto paramSymbol = std::make_shared<ParameterSymbol>(paramName, paramType);
        paramSymbols.push_back(paramSymbol);
    }

    // Create method symbol
    auto methodSymbol = std::make_shared<MethodSymbol>(
        methodName, returnType, paramSymbols, methodDecl->getAccess(), isConstructor);
    currentClass->addMethod(methodSymbol);

    // Analyze method body
    currentMethod = methodSymbol;
    symbolTable.enterScope();

    // Add parameters to scope
    for (const auto& param : paramSymbols) {
        symbolTable.define(param);
    }

    // Analyze the method body
    if (methodDecl->getBody()) {
        for (const auto& stmt : methodDecl->getBody()->getStatements()) {
            analyzeStatement(stmt);
        }
    }

    symbolTable.exitScope();
    currentMethod = nullptr;
}

// Error reporting method
void SemanticAnalyzer::error(const std::string& message, int line, int column) {
    errors.push_back(SemanticError(message, line, column));
}

} // namespace BLang
```

## 4. Handling Common Semantic Challenges

When implementing semantic analysis for an object-oriented language, several challenges need to be addressed:

### 4.1 Method Overriding and Inheritance

In an object-oriented language, we need to verify that method overrides are valid:

- The overriding method must have the same return type (or a subtype)
- The overriding method must not have more restrictive access
- The overriding method must have the same number and types of parameters

```cpp
bool SemanticAnalyzer::checkValidOverride(std::shared_ptr<MethodSymbol> baseMethod,
                                       std::shared_ptr<MethodSymbol> overrideMethod) {
    // Check return type compatibility
    if (!baseMethod->getType()->isAssignableFrom(overrideMethod->getType())) {
        error("Overriding method has incompatible return type", 0, 0);
        return false;
    }

    // Check access level
    if (overrideMethod->getAccess() > baseMethod->getAccess()) {
        error("Overriding method cannot have more restrictive access", 0, 0);
        return false;
    }

    // Check parameter compatibility
    auto baseParams = baseMethod->getParameters();
    auto overrideParams = overrideMethod->getParameters();

    if (baseParams.size() != overrideParams.size()) {
        error("Overriding method must have the same number of parameters", 0, 0);
        return false;
    }

    for (size_t i = 0; i < baseParams.size(); ++i) {
        if (baseParams[i]->getType() != overrideParams[i]->getType()) {
            error("Overriding method has incompatible parameter types", 0, 0);
            return false;
        }
    }

    return true;
}
```

### 4.2 Type Checking and Coercion

Type checking ensures that operations are performed on compatible types:

```cpp
std::shared_ptr<Type> SemanticAnalyzer::analyzeExpression(std::shared_ptr<Expression> expr) {
    if (auto literal = std::dynamic_pointer_cast<LiteralExpr>(expr)) {
        auto token = literal->getToken();

        switch (token.getType()) {
            case TokenType::INTEGER_LITERAL:
                return Type::INT_TYPE;
            case TokenType::FLOAT_LITERAL:
                return Type::FLOAT_TYPE;
            case TokenType::STRING_LITERAL:
                return Type::STRING_TYPE;
            case TokenType::BOOLEAN_LITERAL:
                return Type::BOOLEAN_TYPE;
            case TokenType::NULL_LITERAL:
                return nullptr;  // Null type
            default:
                return Type::VOID_TYPE;
        }
    }

    if (auto binary = std::dynamic_pointer_cast<BinaryExpr>(expr)) {
        auto leftType = analyzeExpression(binary->getLeft());
        auto rightType = analyzeExpression(binary->getRight());
        auto op = binary->getOperator();

        // Arithmetic operators
        if (op == BinaryExpr::Operator::PLUS ||
            op == BinaryExpr::Operator::MINUS ||
            op == BinaryExpr::Operator::MULTIPLY ||
            op == BinaryExpr::Operator::DIVIDE ||
            op == BinaryExpr::Operator::MODULO) {

            // String concatenation with +
            if (op == BinaryExpr::Operator::PLUS &&
                (leftType == Type::STRING_TYPE || rightType == Type::STRING_TYPE)) {
                return Type::STRING_TYPE;
            }

            // Numeric operations
            if ((leftType == Type::INT_TYPE || leftType == Type::FLOAT_TYPE) &&
                (rightType == Type::INT_TYPE || rightType == Type::FLOAT_TYPE)) {
                // Result is float if either operand is float
                if (leftType == Type::FLOAT_TYPE || rightType == Type::FLOAT_TYPE) {
                    return Type::FLOAT_TYPE;
                }
                return Type::INT_TYPE;
            }

            error("Invalid operands for arithmetic operator", 0, 0);
            return Type::VOID_TYPE;
        }

        // Comparison operators
        if (op == BinaryExpr::Operator::LESS ||
            op == BinaryExpr::Operator::LESS_EQUAL ||
            op == BinaryExpr::Operator::GREATER ||
            op == BinaryExpr::Operator::GREATER_EQUAL) {

            if ((leftType == Type::INT_TYPE || leftType == Type::FLOAT_TYPE) &&
                (rightType == Type::INT_TYPE || rightType == Type::FLOAT_TYPE)) {
                return Type::BOOLEAN_TYPE;
            }

            error("Invalid operands for comparison operator", 0, 0);
            return Type::BOOLEAN_TYPE;
        }

        // Equality operators
        if (op == BinaryExpr::Operator::EQUAL ||
            op == BinaryExpr::Operator::NOT_EQUAL) {

            // Check if types are compatible for equality
            if (leftType && rightType &&
                (leftType->isAssignableFrom(rightType) || rightType->isAssignableFrom(leftType))) {
                return Type::BOOLEAN_TYPE;
            }

            error("Invalid operands for equality operator", 0, 0);
            return Type::BOOLEAN_TYPE;
        }

        // Logical operators
        if (op == BinaryExpr::Operator::AND ||
            op == BinaryExpr::Operator::OR) {

            if (leftType == Type::BOOLEAN_TYPE && rightType == Type::BOOLEAN_TYPE) {
                return Type::BOOLEAN_TYPE;
            }

            error("Invalid operands for logical operator", 0, 0);
            return Type::BOOLEAN_TYPE;
        }

        return Type::VOID_TYPE;
    }

    // Additional expression types would be handled here

    return Type::VOID_TYPE;
}
```

### 4.3 Scoping and Variable Resolution

Scoping rules determine variable visibility:

```cpp
std::shared_ptr<VariableSymbol> SemanticAnalyzer::resolveVariable(
    const std::string& name, int line, int column) {

    // First check local scope
    auto symbol = symbolTable.resolve(name);
    if (symbol) {
        if (auto varSymbol = std::dynamic_pointer_cast<VariableSymbol>(symbol)) {
            return varSymbol;
        }
        if (auto paramSymbol = std::dynamic_pointer_cast<ParameterSymbol>(symbol)) {
            // Convert parameter to variable for simplicity
            return std::make_shared<VariableSymbol>(
                paramSymbol->getName(), paramSymbol->getType());
        }
        error("'" + name + "' is not a variable", line, column);
        return nullptr;
    }

    // Then check class fields
    if (currentClass) {
        auto field = currentClass->getField(name);
        if (field) {
            return field;
        }
    }

    error("Variable '" + name + "' not found", line, column);
    return nullptr;
}
```

## 5. Next Steps

With semantic analysis complete, our compiler can now check for type errors and other semantic issues. In the next lesson, we'll build on this foundation to:

1. Design a type system that fully supports object-oriented features
2. Implement type inference to make programming more convenient
3. Handle method overloading based on parameter types
4. Prepare for code generation by attaching type information to AST nodes

## Assignment

1. Complete the semantic analyzer implementation by:

   - Adding the remaining expression analysis methods
   - Implementing statement analysis methods
   - Writing test cases for semantic errors

2. Extend the semantic analyzer to support:

   - Method overloading (multiple methods with the same name but different parameters)
   - Type inference for local variables
   - Array types and operations

3. Implement semantic validation for:
   - Ensuring constructors don't have return types
   - Checking that methods with void return type don't return values
   - Validating that abstract methods are implemented by concrete classes

## Resources

- [Compilers: Principles, Techniques, and Tools](https://www.amazon.com/Compilers-Principles-Techniques-Tools-2nd/dp/0321486811) (Chapter 6: Type Checking)
- [Advanced Compiler Design and Implementation](https://www.amazon.com/Advanced-Compiler-Design-Implementation-Muchnick/dp/1558603204)
- [Type Systems for Programming Languages](http://www.cs.cmu.edu/~rwh/pfpl.html)
