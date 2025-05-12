#pragma once

#include <string>
#include <vector>

namespace blang
{

    // Simple token structure
    struct Token
    {
        enum class Type
        {
            IDENTIFIER,
            NUMBER,
            STRING,
            OPERATOR,
            KEYWORD,
            COMMENT,
            EOF_TOKEN,
            UNKNOWN
        };

        Type type;
        std::string value;
        int line;
        int column;
    };

    class Lexer
    {
    public:
        Lexer();

        // Tokenize input string into a vector of tokens
        std::vector<Token> tokenize(const std::string &source);
    };

} // namespace blang