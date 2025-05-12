# Lesson 3: Syntax Analysis (Parsing)

## Learning Objectives

- Understand context-free grammars and parsing techniques
- Implement a parser using Bison
- Define grammar rules for classes, methods, and expressions
- Build an Abstract Syntax Tree (AST)

## 1. Understanding Syntax Analysis

Syntax analysis is the second phase in the compilation process. It takes the tokens produced by the lexer and determines if they form a valid program according to the grammar rules of the language. The output of this phase is typically an Abstract Syntax Tree (AST), which represents the hierarchical structure of the program.

### 1.1 Context-Free Grammars (CFGs)

Programming languages are typically defined using context-free grammars, which consist of:

- **Terminal symbols**: Tokens from the lexical analysis (keywords, identifiers, literals, etc.)
- **Non-terminal symbols**: Syntactic variables that represent sets of strings
- **Production rules**: Rules that describe how non-terminals can be replaced with combinations of terminals and non-terminals
- **Start symbol**: The non-terminal from which all valid programs are derived

For example, a simple grammar for arithmetic expressions might look like:

```
expr → expr + term | expr - term | term
term → term * factor | term / factor | factor
factor → number | ( expr )
```

### 1.2 Parsing Techniques

Several parsing techniques can be used to analyze the syntax of a program:

1. **Top-down parsing**: Starts from the start symbol and expands production rules to match the input (LL parsers, recursive descent)
2. **Bottom-up parsing**: Starts from the input and reduces sequences of terminals to non-terminals (LR, LALR, SLR parsers)

Bison, which we'll use for BLang, is a bottom-up LALR(1) parser generator. It's particularly well-suited for programming languages because it can handle a wide range of grammar constructs efficiently.

### 1.3 Abstract Syntax Trees (ASTs)

An AST is a tree representation of the abstract syntactic structure of source code. Unlike parse trees, ASTs omit unnecessary syntax details and focus on the semantically relevant parts of the code. For example, an AST for the expression `a = b + c * d` might look like:

```
    AssignNode
      /    \
 VarNode   AddNode
   (a)     /    \
        VarNode  MultiplyNode
          (b)     /    \
               VarNode  VarNode
                 (c)      (d)
```

## 2. Designing the Abstract Syntax Tree for BLang

Before we implement the parser, let's design the AST classes for our language. We'll use a class hierarchy with a common base class for all AST nodes.

### 2.1 Base Node Classes

Create the file `include/AST.h`:

```cpp
#ifndef BLANG_AST_H
#define BLANG_AST_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

namespace BLang {

// Forward declarations
class Type;

// Base class for all AST nodes
class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void print(std::ostream& os, int indent = 0) const = 0;
protected:
    void printIndent(std::ostream& os, int indent) const {
        for (int i = 0; i < indent; ++i) {
            os << "  ";
        }
    }
};

// Base class for expression nodes
class Expression : public ASTNode {
public:
    virtual ~Expression() = default;
    virtual std::shared_ptr<Type> getType() const { return nullptr; }
};

// Base class for statement nodes
class Statement : public ASTNode {
public:
    virtual ~Statement() = default;
};

// Base class for declaration nodes
class Declaration : public ASTNode {
public:
    virtual ~Declaration() = default;
    virtual std::string getName() const = 0;
};

// Program node (root of the AST)
class Program : public ASTNode {
private:
    std::vector<std::shared_ptr<Declaration>> declarations;
public:
    Program(const std::vector<std::shared_ptr<Declaration>>& declarations)
        : declarations(declarations) {}

    const std::vector<std::shared_ptr<Declaration>>& getDeclarations() const {
        return declarations;
    }

    void print(std::ostream& os, int indent = 0) const override {
        os << "Program" << std::endl;
        for (const auto& decl : declarations) {
            decl->print(os, indent + 1);
        }
    }
};

} // namespace BLang

#endif // BLANG_AST_H
```

### 2.2 Expression Nodes

Create the file `include/ExprNodes.h`:

```cpp
#ifndef BLANG_EXPR_NODES_H
#define BLANG_EXPR_NODES_H

#include "AST.h"
#include "Token.h"
#include <memory>

namespace BLang {

// Literal expression node
class LiteralExpr : public Expression {
private:
    Token token;
public:
    LiteralExpr(const Token& token) : token(token) {}

    const Token& getToken() const { return token; }

    void print(std::ostream& os, int indent = 0) const override {
        printIndent(os, indent);
        os << "Literal: " << token.valueToString() << std::endl;
    }
};

// Variable access expression node
class VariableExpr : public Expression {
private:
    std::string name;
public:
    VariableExpr(const std::string& name) : name(name) {}

    const std::string& getName() const { return name; }

    void print(std::ostream& os, int indent = 0) const override {
        printIndent(os, indent);
        os << "Variable: " << name << std::endl;
    }
};

// Assignment expression node
class AssignExpr : public Expression {
private:
    std::shared_ptr<Expression> target;
    std::shared_ptr<Expression> value;
public:
    AssignExpr(std::shared_ptr<Expression> target, std::shared_ptr<Expression> value)
        : target(target), value(value) {}

    std::shared_ptr<Expression> getTarget() const { return target; }
    std::shared_ptr<Expression> getValue() const { return value; }

    void print(std::ostream& os, int indent = 0) const override {
        printIndent(os, indent);
        os << "Assign" << std::endl;
        printIndent(os, indent);
        os << "  Target:" << std::endl;
        target->print(os, indent + 2);
        printIndent(os, indent);
        os << "  Value:" << std::endl;
        value->print(os, indent + 2);
    }
};

// Binary expression node
class BinaryExpr : public Expression {
public:
    enum class Operator {
        PLUS, MINUS, MULTIPLY, DIVIDE, MODULO,
        EQUAL, NOT_EQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
        AND, OR
    };
private:
    std::shared_ptr<Expression> left;
    Operator op;
    std::shared_ptr<Expression> right;
public:
    BinaryExpr(std::shared_ptr<Expression> left, Operator op, std::shared_ptr<Expression> right)
        : left(left), op(op), right(right) {}

    std::shared_ptr<Expression> getLeft() const { return left; }
    Operator getOperator() const { return op; }
    std::shared_ptr<Expression> getRight() const { return right; }

    void print(std::ostream& os, int indent = 0) const override {
        printIndent(os, indent);
        os << "Binary: " << operatorToString(op) << std::endl;
        left->print(os, indent + 1);
        right->print(os, indent + 1);
    }

    static std::string operatorToString(Operator op) {
        switch (op) {
            case Operator::PLUS: return "+";
            case Operator::MINUS: return "-";
            case Operator::MULTIPLY: return "*";
            case Operator::DIVIDE: return "/";
            case Operator::MODULO: return "%";
            case Operator::EQUAL: return "==";
            case Operator::NOT_EQUAL: return "!=";
            case Operator::LESS: return "<";
            case Operator::LESS_EQUAL: return "<=";
            case Operator::GREATER: return ">";
            case Operator::GREATER_EQUAL: return ">=";
            case Operator::AND: return "&&";
            case Operator::OR: return "||";
            default: return "?";
        }
    }
};

// Unary expression node
class UnaryExpr : public Expression {
public:
    enum class Operator {
        NEGATE, NOT
    };
private:
    Operator op;
    std::shared_ptr<Expression> expr;
public:
    UnaryExpr(Operator op, std::shared_ptr<Expression> expr)
        : op(op), expr(expr) {}

    Operator getOperator() const { return op; }
    std::shared_ptr<Expression> getExpression() const { return expr; }

    void print(std::ostream& os, int indent = 0) const override {
        printIndent(os, indent);
        os << "Unary: " << operatorToString(op) << std::endl;
        expr->print(os, indent + 1);
    }

    static std::string operatorToString(Operator op) {
        switch (op) {
            case Operator::NEGATE: return "-";
            case Operator::NOT: return "!";
            default: return "?";
        }
    }
};

// Method call expression node
class CallExpr : public Expression {
private:
    std::shared_ptr<Expression> callee;
    std::vector<std::shared_ptr<Expression>> arguments;
public:
    CallExpr(std::shared_ptr<Expression> callee, const std::vector<std::shared_ptr<Expression>>& arguments)
        : callee(callee), arguments(arguments) {}

    std::shared_ptr<Expression> getCallee() const { return callee; }
    const std::vector<std::shared_ptr<Expression>>& getArguments() const { return arguments; }

    void print(std::ostream& os, int indent = 0) const override {
        printIndent(os, indent);
        os << "Call" << std::endl;
        printIndent(os, indent);
        os << "  Callee:" << std::endl;
        callee->print(os, indent + 2);
        printIndent(os, indent);
        os << "  Arguments:" << std::endl;
        for (const auto& arg : arguments) {
            arg->print(os, indent + 2);
        }
    }
};

// "this" expression node
class ThisExpr : public Expression {
public:
    ThisExpr() {}

    void print(std::ostream& os, int indent = 0) const override {
        printIndent(os, indent);
        os << "This" << std::endl;
    }
};

// "super" expression node
class SuperExpr : public Expression {
private:
    std::string method;
public:
    SuperExpr(const std::string& method) : method(method) {}

    const std::string& getMethod() const { return method; }

    void print(std::ostream& os, int indent = 0) const override {
        printIndent(os, indent);
        os << "Super: " << method << std::endl;
    }
};

// Object creation expression node
class NewExpr : public Expression {
private:
    std::string className;
    std::vector<std::shared_ptr<Expression>> arguments;
public:
    NewExpr(const std::string& className, const std::vector<std::shared_ptr<Expression>>& arguments)
        : className(className), arguments(arguments) {}

    const std::string& getClassName() const { return className; }
    const std::vector<std::shared_ptr<Expression>>& getArguments() const { return arguments; }

    void print(std::ostream& os, int indent = 0) const override {
        printIndent(os, indent);
        os << "New: " << className << std::endl;
        for (const auto& arg : arguments) {
            arg->print(os, indent + 1);
        }
    }
};

// Field access expression node
class GetExpr : public Expression {
private:
    std::shared_ptr<Expression> object;
    std::string name;
public:
    GetExpr(std::shared_ptr<Expression> object, const std::string& name)
        : object(object), name(name) {}

    std::shared_ptr<Expression> getObject() const { return object; }
    const std::string& getName() const { return name; }

    void print(std::ostream& os, int indent = 0) const override {
        printIndent(os, indent);
        os << "Get: " << name << std::endl;
        object->print(os, indent + 1);
    }
};

} // namespace BLang

#endif // BLANG_EXPR_NODES_H
```

### 2.3 Statement Nodes

Create the file `include/StmtNodes.h`:

```cpp
#ifndef BLANG_STMT_NODES_H
#define BLANG_STMT_NODES_H

#include "AST.h"
#include "ExprNodes.h"
#include <memory>
#include <vector>

namespace BLang {

// Expression statement node
class ExpressionStmt : public Statement {
private:
    std::shared_ptr<Expression> expression;
public:
    ExpressionStmt(std::shared_ptr<Expression> expression)
        : expression(expression) {}

    std::shared_ptr<Expression> getExpression() const { return expression; }

    void print(std::ostream& os, int indent = 0) const override {
        printIndent(os, indent);
        os << "Expression Statement" << std::endl;
        expression->print(os, indent + 1);
    }
};

// Variable declaration statement node
class VarStmt : public Statement {
private:
    std::string name;
    std::string typeName;
    std::shared_ptr<Expression> initializer;
public:
    VarStmt(const std::string& name, const std::string& typeName, std::shared_ptr<Expression> initializer)
        : name(name), typeName(typeName), initializer(initializer) {}

    const std::string& getName() const { return name; }
    const std::string& getTypeName() const { return typeName; }
    std::shared_ptr<Expression> getInitializer() const { return initializer; }

    void print(std::ostream& os, int indent = 0) const override {
        printIndent(os, indent);
        os << "Variable Declaration: " << name << " : " << typeName << std::endl;
        if (initializer) {
            printIndent(os, indent);
            os << "  Initializer:" << std::endl;
            initializer->print(os, indent + 2);
        }
    }
};

// Block statement node
class BlockStmt : public Statement {
private:
    std::vector<std::shared_ptr<Statement>> statements;
public:
    BlockStmt(const std::vector<std::shared_ptr<Statement>>& statements)
        : statements(statements) {}

    const std::vector<std::shared_ptr<Statement>>& getStatements() const { return statements; }

    void print(std::ostream& os, int indent = 0) const override {
        printIndent(os, indent);
        os << "Block" << std::endl;
        for (const auto& stmt : statements) {
            stmt->print(os, indent + 1);
        }
    }
};

// If statement node
class IfStmt : public Statement {
private:
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Statement> thenBranch;
    std::shared_ptr<Statement> elseBranch;
public:
    IfStmt(std::shared_ptr<Expression> condition, std::shared_ptr<Statement> thenBranch, std::shared_ptr<Statement> elseBranch)
        : condition(condition), thenBranch(thenBranch), elseBranch(elseBranch) {}

    std::shared_ptr<Expression> getCondition() const { return condition; }
    std::shared_ptr<Statement> getThenBranch() const { return thenBranch; }
    std::shared_ptr<Statement> getElseBranch() const { return elseBranch; }

    void print(std::ostream& os, int indent = 0) const override {
        printIndent(os, indent);
        os << "If Statement" << std::endl;
        printIndent(os, indent);
        os << "  Condition:" << std::endl;
        condition->print(os, indent + 2);
        printIndent(os, indent);
        os << "  Then Branch:" << std::endl;
        thenBranch->print(os, indent + 2);
        if (elseBranch) {
            printIndent(os, indent);
            os << "  Else Branch:" << std::endl;
            elseBranch->print(os, indent + 2);
        }
    }
};

// While statement node
class WhileStmt : public Statement {
private:
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Statement> body;
public:
    WhileStmt(std::shared_ptr<Expression> condition, std::shared_ptr<Statement> body)
        : condition(condition), body(body) {}

    std::shared_ptr<Expression> getCondition() const { return condition; }
    std::shared_ptr<Statement> getBody() const { return body; }

    void print(std::ostream& os, int indent = 0) const override {
        printIndent(os, indent);
        os << "While Statement" << std::endl;
        printIndent(os, indent);
        os << "  Condition:" << std::endl;
        condition->print(os, indent + 2);
        printIndent(os, indent);
        os << "  Body:" << std::endl;
        body->print(os, indent + 2);
    }
};

// For statement node
class ForStmt : public Statement {
private:
    std::shared_ptr<Statement> initializer;
    std::shared_ptr<Expression> condition;
    std::shared_ptr<Expression> increment;
    std::shared_ptr<Statement> body;
public:
    ForStmt(std::shared_ptr<Statement> initializer, std::shared_ptr<Expression> condition,
            std::shared_ptr<Expression> increment, std::shared_ptr<Statement> body)
        : initializer(initializer), condition(condition), increment(increment), body(body) {}

    std::shared_ptr<Statement> getInitializer() const { return initializer; }
    std::shared_ptr<Expression> getCondition() const { return condition; }
    std::shared_ptr<Expression> getIncrement() const { return increment; }
    std::shared_ptr<Statement> getBody() const { return body; }

    void print(std::ostream& os, int indent = 0) const override {
        printIndent(os, indent);
        os << "For Statement" << std::endl;
        if (initializer) {
            printIndent(os, indent);
            os << "  Initializer:" << std::endl;
            initializer->print(os, indent + 2);
        }
        if (condition) {
            printIndent(os, indent);
            os << "  Condition:" << std::endl;
            condition->print(os, indent + 2);
        }
        if (increment) {
            printIndent(os, indent);
            os << "  Increment:" << std::endl;
            increment->print(os, indent + 2);
        }
        printIndent(os, indent);
        os << "  Body:" << std::endl;
        body->print(os, indent + 2);
    }
};

// Return statement node
class ReturnStmt : public Statement {
private:
    std::shared_ptr<Expression> value;
public:
    ReturnStmt(std::shared_ptr<Expression> value)
        : value(value) {}

    std::shared_ptr<Expression> getValue() const { return value; }

    void print(std::ostream& os, int indent = 0) const override {
        printIndent(os, indent);
        os << "Return Statement" << std::endl;
        if (value) {
            value->print(os, indent + 1);
        }
    }
};

} // namespace BLang

#endif // BLANG_STMT_NODES_H
```

### 2.4 Declaration Nodes

Create the file `include/DeclNodes.h`:

```cpp
#ifndef BLANG_DECL_NODES_H
#define BLANG_DECL_NODES_H

#include "AST.h"
#include "StmtNodes.h"
#include <memory>
#include <vector>
#include <string>

namespace BLang {

// Enum for access modifiers
enum class AccessModifier {
    PUBLIC,
    PRIVATE,
    PROTECTED
};

// Variable declaration node
class VarDecl : public Declaration {
private:
    std::string name;
    std::string typeName;
    AccessModifier access;
    std::shared_ptr<Expression> initializer;
public:
    VarDecl(const std::string& name, const std::string& typeName, AccessModifier access,
           std::shared_ptr<Expression> initializer)
        : name(name), typeName(typeName), access(access), initializer(initializer) {}

    std::string getName() const override { return name; }
    const std::string& getTypeName() const { return typeName; }
    AccessModifier getAccess() const { return access; }
    std::shared_ptr<Expression> getInitializer() const { return initializer; }

    void print(std::ostream& os, int indent = 0) const override {
        printIndent(os, indent);
        os << "Variable Declaration: " << accessModifierToString(access) << " "
           << typeName << " " << name << std::endl;
        if (initializer) {
            printIndent(os, indent);
            os << "  Initializer:" << std::endl;
            initializer->print(os, indent + 2);
        }
    }

    static std::string accessModifierToString(AccessModifier access) {
        switch (access) {
            case AccessModifier::PUBLIC: return "public";
            case AccessModifier::PRIVATE: return "private";
            case AccessModifier::PROTECTED: return "protected";
            default: return "unknown";
        }
    }
};

// Function parameter node
class Parameter {
private:
    std::string name;
    std::string typeName;
public:
    Parameter(const std::string& name, const std::string& typeName)
        : name(name), typeName(typeName) {}

    const std::string& getName() const { return name; }
    const std::string& getTypeName() const { return typeName; }
};

// Method declaration node
class MethodDecl : public Declaration {
private:
    std::string name;
    std::string returnTypeName;
    std::vector<Parameter> parameters;
    std::shared_ptr<BlockStmt> body;
    AccessModifier access;
    bool isConstructor;
public:
    MethodDecl(const std::string& name, const std::string& returnTypeName,
              const std::vector<Parameter>& parameters, std::shared_ptr<BlockStmt> body,
              AccessModifier access, bool isConstructor = false)
        : name(name), returnTypeName(returnTypeName), parameters(parameters), body(body),
          access(access), isConstructor(isConstructor) {}

    std::string getName() const override { return name; }
    const std::string& getReturnTypeName() const { return returnTypeName; }
    const std::vector<Parameter>& getParameters() const { return parameters; }
    std::shared_ptr<BlockStmt> getBody() const { return body; }
    AccessModifier getAccess() const { return access; }
    bool getIsConstructor() const { return isConstructor; }

    void print(std::ostream& os, int indent = 0) const override {
        printIndent(os, indent);
        os << "Method Declaration: " << VarDecl::accessModifierToString(access) << " "
           << returnTypeName << " " << name << "(";

        for (size_t i = 0; i < parameters.size(); ++i) {
            os << parameters[i].getTypeName() << " " << parameters[i].getName();
            if (i < parameters.size() - 1) {
                os << ", ";
            }
        }

        os << ")" << std::endl;

        if (body) {
            body->print(os, indent + 1);
        }
    }
};

// Class declaration node
class ClassDecl : public Declaration {
private:
    std::string name;
    std::string superClass;
    std::vector<std::shared_ptr<VarDecl>> fields;
    std::vector<std::shared_ptr<MethodDecl>> methods;
public:
    ClassDecl(const std::string& name, const std::string& superClass,
             const std::vector<std::shared_ptr<VarDecl>>& fields,
             const std::vector<std::shared_ptr<MethodDecl>>& methods)
        : name(name), superClass(superClass), fields(fields), methods(methods) {}

    std::string getName() const override { return name; }
    const std::string& getSuperClass() const { return superClass; }
    const std::vector<std::shared_ptr<VarDecl>>& getFields() const { return fields; }
    const std::vector<std::shared_ptr<MethodDecl>>& getMethods() const { return methods; }

    void print(std::ostream& os, int indent = 0) const override {
        printIndent(os, indent);
        os << "Class Declaration: " << name;
        if (!superClass.empty()) {
            os << " extends " << superClass;
        }
        os << std::endl;

        printIndent(os, indent);
        os << "  Fields:" << std::endl;
        for (const auto& field : fields) {
            field->print(os, indent + 2);
        }

        printIndent(os, indent);
        os << "  Methods:" << std::endl;
        for (const auto& method : methods) {
            method->print(os, indent + 2);
        }
    }
};

} // namespace BLang

#endif // BLANG_DECL_NODES_H
```
