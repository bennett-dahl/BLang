# Lesson 9: Standard Library & Built-ins

## Learning Objectives

- Design and implement a minimal standard library for BLang
- Create built-in methods for core types (String, Number, Boolean, etc.)
- Implement an I/O system for file and console operations
- Build collection classes (List, Map, Set) with common operations

## 1. Introduction to Standard Libraries

Every programming language needs a standard library to be practical and useful. The standard library provides essential functionality that programmers can rely on without having to implement from scratch. A well-designed standard library makes a language more productive and enjoyable to use.

### 1.1 Goals of a Standard Library

When designing a standard library for BLang, we have several goals:

1. **Consistency**: The standard library should follow consistent design patterns and naming conventions
2. **Simplicity**: Core functionality should be easy to use
3. **Efficiency**: Standard operations should be implemented efficiently
4. **Extensibility**: Users should be able to extend and customize standard library components

### 1.2 Core Components of a Standard Library

Our BLang standard library will include the following components:

- **Core types**: Enhanced functionality for primitive types
- **Collections**: Data structures for storing and manipulating groups of objects
- **I/O**: File system access and console input/output
- **String utilities**: String manipulation and formatting
- **Math utilities**: Advanced mathematical operations
- **Date/Time**: Date and time representation and manipulation

## 2. Building Core Types

Let's start by implementing core classes that enhance primitive types with useful methods.

### 2.1 The Object Class

In BLang, every class implicitly inherits from `Object`, the root of the class hierarchy:

```cpp
// In ClassDeclaration.h
class ClassDeclaration {
private:
    // ...existing code...

public:
    bool isObjectClass() const {
        return name == "Object" && superClassName.empty();
    }
};

// In ClassInstance.h
class ObjectClass {
public:
    static std::shared_ptr<ClassInstance> create() {
        auto objectClass = std::make_shared<ClassInstance>("Object");

        // Add basic Object methods
        addToStringMethod(objectClass);
        addEqualsMethod(objectClass);
        addHashCodeMethod(objectClass);

        return objectClass;
    }

private:
    static void addToStringMethod(std::shared_ptr<ClassInstance> objectClass) {
        // Implementation of Object.toString
        auto toStringImpl = std::make_shared<IR::Function>("Object.toString");

        // Build basic blocks and instructions
        auto entry = std::make_shared<IR::BasicBlock>("entry");

        // Format: "ClassName@HashCode"
        auto className = std::make_shared<IR::GetClass>();
        auto getClassName = std::make_shared<IR::GetClassName>();
        auto hashCode = std::make_shared<IR::CallMethod>(
            std::make_shared<IR::Value>("this"),
            std::make_shared<IR::Value>("hashCode"),
            std::make_shared<IR::Value>("0") // No arguments
        );

        auto formatStr = std::make_shared<IR::Concat>(
            getClassName,
            std::make_shared<IR::Value>("@")
        );
        auto formatStrWithHash = std::make_shared<IR::Concat>(
            formatStr,
            hashCode
        );

        auto returnInst = std::make_shared<IR::Return>(formatStrWithHash);

        // Add instructions to basic block
        entry->addInstruction(className);
        entry->addInstruction(getClassName);
        entry->addInstruction(hashCode);
        entry->addInstruction(formatStr);
        entry->addInstruction(formatStrWithHash);
        entry->addInstruction(returnInst);

        // Add basic block to function
        toStringImpl->addBasicBlock(entry);

        // Create method instance
        auto toStringMethod = std::make_shared<MethodInstance>(
            "toString",
            toStringImpl,
            objectClass,
            std::vector<std::string>{},
            true // virtual
        );

        // Add method to object class
        objectClass->addMethod("toString", toStringMethod);
    }

    static void addEqualsMethod(std::shared_ptr<ClassInstance> objectClass) {
        // Implementation of Object.equals
        // Default: reference equality
        auto equalsImpl = std::make_shared<IR::Function>("Object.equals");

        // Add parameter
        equalsImpl->addParameter("obj");

        auto entry = std::make_shared<IR::BasicBlock>("entry");

        // Reference equality check
        auto thisRef = std::make_shared<IR::Value>("this");
        auto objParam = std::make_shared<IR::Value>("obj");
        auto refEqual = std::make_shared<IR::RefEquals>(thisRef, objParam);
        auto returnInst = std::make_shared<IR::Return>(refEqual);

        entry->addInstruction(refEqual);
        entry->addInstruction(returnInst);

        equalsImpl->addBasicBlock(entry);

        auto equalsMethod = std::make_shared<MethodInstance>(
            "equals",
            equalsImpl,
            objectClass,
            std::vector<std::string>{"obj"},
            true
        );

        objectClass->addMethod("equals", equalsMethod);
    }

    static void addHashCodeMethod(std::shared_ptr<ClassInstance> objectClass) {
        // Implementation of Object.hashCode
        // Default: memory address-based hash
        auto hashCodeImpl = std::make_shared<IR::Function>("Object.hashCode");

        auto entry = std::make_shared<IR::BasicBlock>("entry");

        auto thisRef = std::make_shared<IR::Value>("this");
        auto addressHash = std::make_shared<IR::AddressHash>(thisRef);
        auto returnInst = std::make_shared<IR::Return>(addressHash);

        entry->addInstruction(addressHash);
        entry->addInstruction(returnInst);

        hashCodeImpl->addBasicBlock(entry);

        auto hashCodeMethod = std::make_shared<MethodInstance>(
            "hashCode",
            hashCodeImpl,
            objectClass,
            std::vector<std::string>{},
            true
        );

        objectClass->addMethod("hashCode", hashCodeMethod);
    }
};
```

### 2.2 String Class

Now, let's implement the String class with common string operations:

```cpp
class StringClass {
public:
    static std::shared_ptr<ClassInstance> create(
        std::shared_ptr<ClassInstance> objectClass) {

        auto stringClass = std::make_shared<ClassInstance>("String", objectClass);

        // Add String methods
        addLengthMethod(stringClass);
        addConcatMethod(stringClass);
        addSubstringMethod(stringClass);
        addIndexOfMethod(stringClass);
        addToUpperMethod(stringClass);
        addToLowerMethod(stringClass);
        addEqualsMethod(stringClass);  // Override equals for string content equality

        return stringClass;
    }

private:
    static void addLengthMethod(std::shared_ptr<ClassInstance> stringClass) {
        // Implementation of String.length
        auto lengthImpl = std::make_shared<IR::Function>("String.length");

        auto entry = std::make_shared<IR::BasicBlock>("entry");

        auto thisString = std::make_shared<IR::Value>("this");
        auto stringLength = std::make_shared<IR::StringLength>(thisString);
        auto returnInst = std::make_shared<IR::Return>(stringLength);

        entry->addInstruction(stringLength);
        entry->addInstruction(returnInst);

        lengthImpl->addBasicBlock(entry);

        auto lengthMethod = std::make_shared<MethodInstance>(
            "length",
            lengthImpl,
            stringClass,
            std::vector<std::string>{},
            false  // Non-virtual for efficiency
        );

        stringClass->addMethod("length", lengthMethod);
    }

    // More string methods here...

    static void addConcatMethod(std::shared_ptr<ClassInstance> stringClass) {
        // Implementation of String.concat
        auto concatImpl = std::make_shared<IR::Function>("String.concat");

        // Add parameter
        concatImpl->addParameter("str");

        auto entry = std::make_shared<IR::BasicBlock>("entry");

        auto thisString = std::make_shared<IR::Value>("this");
        auto otherString = std::make_shared<IR::Value>("str");

        // Convert to string if needed
        auto toString = std::make_shared<IR::ToString>(otherString);
        auto concat = std::make_shared<IR::Concat>(thisString, toString);
        auto returnInst = std::make_shared<IR::Return>(concat);

        entry->addInstruction(toString);
        entry->addInstruction(concat);
        entry->addInstruction(returnInst);

        concatImpl->addBasicBlock(entry);

        auto concatMethod = std::make_shared<MethodInstance>(
            "concat",
            concatImpl,
            stringClass,
            std::vector<std::string>{"str"},
            false
        );

        stringClass->addMethod("concat", concatMethod);
    }

    static void addEqualsMethod(std::shared_ptr<ClassInstance> stringClass) {
        // Override equals for content-based equality
        auto equalsImpl = std::make_shared<IR::Function>("String.equals");

        // Add parameter
        equalsImpl->addParameter("obj");

        auto entry = std::make_shared<IR::BasicBlock>("entry");
        auto isString = std::make_shared<IR::BasicBlock>("isString");
        auto notString = std::make_shared<IR::BasicBlock>("notString");

        // Check if obj is a String
        auto objParam = std::make_shared<IR::Value>("obj");
        auto objClass = std::make_shared<IR::GetClass>(objParam);
        auto stringClassRef = std::make_shared<IR::Value>("String");
        auto checkIsString = std::make_shared<IR::InstanceOf>(objParam, stringClassRef);
        auto branchInst = std::make_shared<IR::Branch>(checkIsString,
                                                    isString->getName(),
                                                    notString->getName());

        entry->addInstruction(objClass);
        entry->addInstruction(checkIsString);
        entry->addInstruction(branchInst);

        // If obj is a String, compare content
        auto thisString = std::make_shared<IR::Value>("this");
        auto stringCompare = std::make_shared<IR::StringEquals>(thisString, objParam);
        auto returnTrue = std::make_shared<IR::Return>(stringCompare);

        isString->addInstruction(stringCompare);
        isString->addInstruction(returnTrue);

        // If obj is not a String, return false
        auto returnFalse = std::make_shared<IR::Return>(
            std::make_shared<IR::Value>("false")
        );

        notString->addInstruction(returnFalse);

        equalsImpl->addBasicBlock(entry);
        equalsImpl->addBasicBlock(isString);
        equalsImpl->addBasicBlock(notString);

        auto equalsMethod = std::make_shared<MethodInstance>(
            "equals",
            equalsImpl,
            stringClass,
            std::vector<std::string>{"obj"},
            true
        );

        stringClass->addMethod("equals", equalsMethod);
    }

    // Additional String methods would be implemented similarly
};
```

## 3. Building Collection Classes

Collections are essential for any programming language. Let's implement the core collection classes for BLang.

### 3.1 The List Class

First, let's implement a dynamic array implementation called List:

```cpp
class ListClass {
public:
    static std::shared_ptr<ClassInstance> create(
        std::shared_ptr<ClassInstance> objectClass) {

        auto listClass = std::make_shared<ClassInstance>("List", objectClass);

        // Add List fields
        listClass->addField("elements");  // Array of elements
        listClass->addField("size");      // Current size
        listClass->addField("capacity");  // Current capacity

        // Add List methods
        addConstructor(listClass);
        addSizeMethod(listClass);
        addGetMethod(listClass);
        addSetMethod(listClass);
        addAddMethod(listClass);
        addRemoveMethod(listClass);
        addContainsMethod(listClass);

        return listClass;
    }

private:
    static void addConstructor(std::shared_ptr<ClassInstance> listClass) {
        // Implementation of List constructor
        auto ctorImpl = std::make_shared<IR::Function>("List.constructor");

        auto entry = std::make_shared<IR::BasicBlock>("entry");

        // Initialize fields
        auto thisRef = std::make_shared<IR::Value>("this");
        auto initArray = std::make_shared<IR::NewArray>(
            std::make_shared<IR::Value>("10")  // Initial capacity
        );
        auto setElements = std::make_shared<IR::SetField>(
            thisRef,
            std::make_shared<IR::Value>("elements"),
            initArray
        );

        auto setSize = std::make_shared<IR::SetField>(
            thisRef,
            std::make_shared<IR::Value>("size"),
            std::make_shared<IR::Value>("0")
        );

        auto setCapacity = std::make_shared<IR::SetField>(
            thisRef,
            std::make_shared<IR::Value>("capacity"),
            std::make_shared<IR::Value>("10")
        );

        auto returnVoid = std::make_shared<IR::Return>();

        entry->addInstruction(initArray);
        entry->addInstruction(setElements);
        entry->addInstruction(setSize);
        entry->addInstruction(setCapacity);
        entry->addInstruction(returnVoid);

        ctorImpl->addBasicBlock(entry);

        auto ctorMethod = std::make_shared<MethodInstance>(
            "constructor",
            ctorImpl,
            listClass,
            std::vector<std::string>{},
            false
        );

        listClass->addMethod("constructor", ctorMethod);
    }

    static void addSizeMethod(std::shared_ptr<ClassInstance> listClass) {
        // Implementation of List.size
        auto sizeImpl = std::make_shared<IR::Function>("List.size");

        auto entry = std::make_shared<IR::BasicBlock>("entry");

        auto thisRef = std::make_shared<IR::Value>("this");
        auto getSize = std::make_shared<IR::GetField>(
            thisRef,
            std::make_shared<IR::Value>("size")
        );
        auto returnSize = std::make_shared<IR::Return>(getSize);

        entry->addInstruction(getSize);
        entry->addInstruction(returnSize);

        sizeImpl->addBasicBlock(entry);

        auto sizeMethod = std::make_shared<MethodInstance>(
            "size",
            sizeImpl,
            listClass,
            std::vector<std::string>{},
            false
        );

        listClass->addMethod("size", sizeMethod);
    }

    // Additional List methods would be implemented similarly
};
```

## 4. Implementing I/O

I/O functionality is essential for any practical programming language. Let's implement basic I/O classes for BLang.

### 4.1 Console I/O

First, let's implement console input/output:

```cpp
class ConsoleClass {
public:
    static std::shared_ptr<ClassInstance> create(
        std::shared_ptr<ClassInstance> objectClass) {

        auto consoleClass = std::make_shared<ClassInstance>("Console", objectClass);

        // Add Console methods (all static)
        addPrintMethod(consoleClass);
        addPrintlnMethod(consoleClass);
        addReadLineMethod(consoleClass);
        addReadNumberMethod(consoleClass);

        return consoleClass;
    }

private:
    static void addPrintMethod(std::shared_ptr<ClassInstance> consoleClass) {
        // Implementation of Console.print (static method)
        auto printImpl = std::make_shared<IR::Function>("Console.print");

        // Add parameter
        printImpl->addParameter("message");

        auto entry = std::make_shared<IR::BasicBlock>("entry");

        auto message = std::make_shared<IR::Value>("message");
        auto toString = std::make_shared<IR::ToString>(message);
        auto printOp = std::make_shared<IR::Print>(toString);
        auto returnVoid = std::make_shared<IR::Return>();

        entry->addInstruction(toString);
        entry->addInstruction(printOp);
        entry->addInstruction(returnVoid);

        printImpl->addBasicBlock(entry);

        auto printMethod = std::make_shared<MethodInstance>(
            "print",
            printImpl,
            consoleClass,
            std::vector<std::string>{"message"},
            false
        );

        // Mark as static method
        printMethod->setStatic(true);

        consoleClass->addMethod("print", printMethod);
    }

    static void addPrintlnMethod(std::shared_ptr<ClassInstance> consoleClass) {
        // Implementation of Console.println (static method)
        auto printlnImpl = std::make_shared<IR::Function>("Console.println");

        // Add parameter
        printlnImpl->addParameter("message");

        auto entry = std::make_shared<IR::BasicBlock>("entry");

        auto message = std::make_shared<IR::Value>("message");
        auto toString = std::make_shared<IR::ToString>(message);
        auto printlnOp = std::make_shared<IR::Println>(toString);
        auto returnVoid = std::make_shared<IR::Return>();

        entry->addInstruction(toString);
        entry->addInstruction(printlnOp);
        entry->addInstruction(returnVoid);

        printlnImpl->addBasicBlock(entry);

        auto printlnMethod = std::make_shared<MethodInstance>(
            "println",
            printlnImpl,
            consoleClass,
            std::vector<std::string>{"message"},
            false
        );

        // Mark as static method
        printlnMethod->setStatic(true);

        consoleClass->addMethod("println", printlnMethod);
    }

    // Additional Console methods would be implemented similarly
};
```

### 4.2 File I/O

Next, let's implement basic file I/O:

```cpp
class FileClass {
public:
    static std::shared_ptr<ClassInstance> create(
        std::shared_ptr<ClassInstance> objectClass) {

        auto fileClass = std::make_shared<ClassInstance>("File", objectClass);

        // Add File fields
        fileClass->addField("path");      // File path
        fileClass->addField("handle");    // Native file handle
        fileClass->addField("isOpen");    // Whether the file is open

        // Add File methods
        addConstructor(fileClass);
        addOpenMethod(fileClass);
        addCloseMethod(fileClass);
        addReadMethod(fileClass);
        addWriteMethod(fileClass);
        addExistsMethod(fileClass);

        return fileClass;
    }

private:
    static void addConstructor(std::shared_ptr<ClassInstance> fileClass) {
        // Implementation of File constructor
        auto ctorImpl = std::make_shared<IR::Function>("File.constructor");

        // Add parameter
        ctorImpl->addParameter("path");

        auto entry = std::make_shared<IR::BasicBlock>("entry");

        auto thisRef = std::make_shared<IR::Value>("this");
        auto pathParam = std::make_shared<IR::Value>("path");

        auto setPath = std::make_shared<IR::SetField>(
            thisRef,
            std::make_shared<IR::Value>("path"),
            pathParam
        );

        auto setHandle = std::make_shared<IR::SetField>(
            thisRef,
            std::make_shared<IR::Value>("handle"),
            std::make_shared<IR::Value>("null")
        );

        auto setIsOpen = std::make_shared<IR::SetField>(
            thisRef,
            std::make_shared<IR::Value>("isOpen"),
            std::make_shared<IR::Value>("false")
        );

        auto returnVoid = std::make_shared<IR::Return>();

        entry->addInstruction(setPath);
        entry->addInstruction(setHandle);
        entry->addInstruction(setIsOpen);
        entry->addInstruction(returnVoid);

        ctorImpl->addBasicBlock(entry);

        auto ctorMethod = std::make_shared<MethodInstance>(
            "constructor",
            ctorImpl,
            fileClass,
            std::vector<std::string>{"path"},
            false
        );

        fileClass->addMethod("constructor", ctorMethod);
    }

    // Additional File methods would be implemented similarly
};
```

## 5. Math & Utility Classes

Let's implement some utility classes to round out our standard library.

### 5.1 Math Class

The Math class provides common mathematical operations:

```cpp
class MathClass {
public:
    static std::shared_ptr<ClassInstance> create(
        std::shared_ptr<ClassInstance> objectClass) {

        auto mathClass = std::make_shared<ClassInstance>("Math", objectClass);

        // Add Math methods (all static)
        addAbsMethod(mathClass);
        addMinMethod(mathClass);
        addMaxMethod(mathClass);
        addSqrtMethod(mathClass);
        addPowMethod(mathClass);
        addRandomMethod(mathClass);

        return mathClass;
    }

private:
    static void addAbsMethod(std::shared_ptr<ClassInstance> mathClass) {
        // Implementation of Math.abs (static method)
        auto absImpl = std::make_shared<IR::Function>("Math.abs");

        // Add parameter
        absImpl->addParameter("value");

        auto entry = std::make_shared<IR::BasicBlock>("entry");
        auto lessThanZero = std::make_shared<IR::BasicBlock>("lessThanZero");
        auto greaterOrEqualZero = std::make_shared<IR::BasicBlock>("greaterOrEqualZero");

        auto value = std::make_shared<IR::Value>("value");
        auto zero = std::make_shared<IR::Value>("0");
        auto compare = std::make_shared<IR::LessThan>(value, zero);
        auto branchInst = std::make_shared<IR::Branch>(
            compare,
            lessThanZero->getName(),
            greaterOrEqualZero->getName()
        );

        entry->addInstruction(compare);
        entry->addInstruction(branchInst);

        // If value < 0, negate it
        auto negate = std::make_shared<IR::Negate>(value);
        auto returnNegated = std::make_shared<IR::Return>(negate);

        lessThanZero->addInstruction(negate);
        lessThanZero->addInstruction(returnNegated);

        // If value >= 0, return as is
        auto returnValue = std::make_shared<IR::Return>(value);

        greaterOrEqualZero->addInstruction(returnValue);

        absImpl->addBasicBlock(entry);
        absImpl->addBasicBlock(lessThanZero);
        absImpl->addBasicBlock(greaterOrEqualZero);

        auto absMethod = std::make_shared<MethodInstance>(
            "abs",
            absImpl,
            mathClass,
            std::vector<std::string>{"value"},
            false
        );

        // Mark as static method
        absMethod->setStatic(true);

        mathClass->addMethod("abs", absMethod);
    }

    // Additional Math methods would be implemented similarly
};
```

## 6. Initializing the Standard Library

Now, let's create a function to initialize the entire standard library:

```cpp
class StandardLibrary {
public:
    static void initialize(VirtualMachine& vm) {
        // Create the Object class (root of class hierarchy)
        auto objectClass = ObjectClass::create();
        vm.registerClass(objectClass);

        // Create standard types
        auto stringClass = StringClass::create(objectClass);
        vm.registerClass(stringClass);

        auto numberClass = NumberClass::create(objectClass);
        vm.registerClass(numberClass);

        auto booleanClass = BooleanClass::create(objectClass);
        vm.registerClass(booleanClass);

        // Create collection classes
        auto listClass = ListClass::create(objectClass);
        vm.registerClass(listClass);

        auto mapClass = MapClass::create(objectClass);
        vm.registerClass(mapClass);

        // Create I/O classes
        auto consoleClass = ConsoleClass::create(objectClass);
        vm.registerClass(consoleClass);

        auto fileClass = FileClass::create(objectClass);
        vm.registerClass(fileClass);

        // Create utility classes
        auto mathClass = MathClass::create(objectClass);
        vm.registerClass(mathClass);

        auto dateClass = DateClass::create(objectClass);
        vm.registerClass(dateClass);
    }
};
```

## 7. Using the Standard Library

Here's how a BLang program might use the standard library:

```
// Sample BLang program using the standard library
class Main {
    static func main() {
        Console.println("Hello, BLang!");

        // Create a list
        var list = new List();
        list.add("Apple");
        list.add("Banana");
        list.add("Cherry");

        // Iterate over the list
        for (var i = 0; i < list.size(); i++) {
            Console.println("Fruit: " + list.get(i));
        }

        // Use Math functions
        var x = 10;
        var y = -5;
        Console.println("Max: " + Math.max(x, y));
        Console.println("Abs: " + Math.abs(y));

        // File I/O
        var file = new File("data.txt");
        if (file.exists()) {
            file.open("r");
            var content = file.read();
            file.close();
            Console.println("File content: " + content);
        } else {
            Console.println("File not found!");
        }
    }
}
```

## 8. Exercises and Assignments

Now that you've implemented a standard library for BLang, try these exercises to deepen your understanding:

1. **Extend the String Class**: Add methods for case-insensitive comparison, pattern matching, and string splitting.

2. **Create a Regular Expression Class**: Implement a RegExp class with pattern matching and substitution.

3. **Implement Thread Support**: Add basic threading and synchronization primitives to BLang.

4. **Add a Network Library**: Implement basic networking functionality (TCP/IP sockets, HTTP client).

5. **Build a JSON Parser/Serializer**: Create classes for working with JSON data in BLang.

## 9. Conclusion

In this lesson, we've built a comprehensive standard library for BLang, including:

- Core types with enhanced functionality
- Collection classes for working with groups of objects
- I/O classes for file and console operations
- Utility classes for common operations

A well-designed standard library makes your language much more usable and powerful. In the next and final lesson, we'll bring everything together and create a complete BLang implementation that users can deploy and run.
