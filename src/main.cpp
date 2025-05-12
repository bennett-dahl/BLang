#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <blang/lexer.h>

int main(int argc, char *argv[])
{
    std::cout << "BLang - Object-Oriented Programming Language" << std::endl;
    std::cout << "Version 0.1.0" << std::endl;

    // Basic command-line handling
    if (argc < 2)
    {
        // No arguments, run REPL
        std::cout << "Interactive mode not implemented yet." << std::endl;
        return 0;
    }

    std::string arg1 = argv[1];
    if (arg1 == "-h" || arg1 == "--help")
    {
        std::cout << "Usage: blang [options] [script]" << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  -h, --help     Show this help message" << std::endl;
        std::cout << "  -v, --version  Show version information" << std::endl;
        return 0;
    }
    else if (arg1 == "-v" || arg1 == "--version")
    {
        // Version already shown
        return 0;
    }
    else
    {
        // Treat as a script file
        std::cout << "Attempting to read file: " << arg1 << std::endl;

        // Try to read the file
        std::ifstream file(arg1);
        if (!file.is_open())
        {
            std::cerr << "Error: Could not open file " << arg1 << std::endl;
            return 1;
        }

        // Read the entire file content
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        // Create a lexer and tokenize the input
        blang::Lexer lexer;
        auto tokens = lexer.tokenize(source);

        std::cout << "Tokenized " << tokens.size() - 1 << " tokens (plus EOF)" << std::endl;

        return 0;
    }
}