# BLang - Object-Oriented Programming Language

This repository contains both the implementation of the BLang programming language and an educational website teaching how to build it from scratch.

## Project Structure

- `/` - Root directory for the BLang language implementation
- `/website` - Contains the educational website
  - `/website/lessons` - Markdown files with the course content
  - `/website/index.html` - Main webpage
  - `/website/styles.css` - Stylesheet
  - `/website/script.js` - JavaScript for the website

## BLang Language Implementation

This directory is intended for the actual implementation of the BLang programming language as you follow along with the course. As you progress through the lessons, you'll build:

- Lexer for tokenizing source code
- Parser for creating an abstract syntax tree
- Type checker for semantic analysis
- Intermediate representation for optimization
- Interpreter/compiler for execution
- Runtime system with memory management
- Standard library with core functionality

## Educational Website

The `/website` directory contains a complete course on building an object-oriented programming language from scratch. The course is organized into 10 comprehensive lessons.

### How to View the Course Website

1. Navigate to the `website` directory
2. Start a local web server:
   ```
   python -m http.server    # If you have Python 3
   ```
   or
   ```
   npx http-server          # If you have Node.js
   ```
3. Open your browser to `http://localhost:8000`

### Course Contents

1. **Foundations**: Language design principles and project setup
2. **Lexical Analysis**: Tokenizing source code
3. **Syntax Analysis**: Parsing and abstract syntax trees
4. **Semantic Analysis**: Type checking and validation
5. **Type System**: Type hierarchies and relationships
6. **Intermediate Representation**: IR generation and optimization
7. **Interpreter**: Virtual machine implementation
8. **Runtime & Memory Management**: Object system and garbage collection
9. **Standard Library**: Building essential language features
10. **Final Integration**: Bringing everything together

## Getting Started

1. Review the course website to understand the concepts
2. Follow along with each lesson by implementing the corresponding parts in the root directory
3. Experiment and extend the language with your own features

## Prerequisites

- Knowledge of C++ programming
- Basic understanding of compiler/interpreter concepts
- A C++ development environment with CMake
