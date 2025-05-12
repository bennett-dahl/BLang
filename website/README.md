# BLang Course Website

A comprehensive educational website for learning how to build an object-oriented programming language from scratch.

## Folder Structure

- `/website` - Contains all website files
  - `/website/lessons` - Contains all lesson markdown files
  - `/website/index.html` - Main HTML file
  - `/website/styles.css` - CSS stylesheet
  - `/website/script.js` - JavaScript code
  - `/website/README.md` - This file

## Getting Started

### Local Development

1. Navigate to the `website` directory
2. Start a local web server in this directory:
   - If you have Python installed:
     - Python 3: `python -m http.server`
     - Python 2: `python -m SimpleHTTPServer`
   - If you have Node.js installed:
     - Install `http-server` globally: `npm install -g http-server`
     - Run: `http-server`
3. Open your browser and navigate to `http://localhost:8000` (or the port shown in your terminal)

### Using Without a Web Server

Due to browser security restrictions when loading local files, some features may not work if you simply open the HTML file directly. It's recommended to use a local web server.

However, if you need to run without a server, you can:

1. Open `index.html` directly in your browser
2. If content doesn't load, you may need to adjust your browser's security settings or use a web server instead

## Features

- Dark mode UI for comfortable reading
- Navigation sidebar with all lessons
- Code syntax highlighting
- Responsive design for both desktop and mobile

## Course Content

The course covers the complete process of building a programming language:

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

Each lesson includes theory, implementation code, and practical exercises.

## Technologies Used

- HTML5, CSS3, and JavaScript
- [marked.js](https://marked.js.org/) for Markdown parsing
- [highlight.js](https://highlightjs.org/) for syntax highlighting

## License

This educational content is provided for learning purposes.
