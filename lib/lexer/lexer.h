#ifndef ZAP_LEXER_H
#define ZAP_LEXER_H

#include <string>
#include <unordered_map>

#include "Token.h"

class Lexer {
public:
    explicit Lexer(std::string& input);

    Token::Token NextToken();

private:
    void SkipWhitespaces();

    std::string ReadIdent();

    std::string ReadNumber();

    bool ReadChar();

    std::string _input;
    std::string::iterator _curChar;
    std::string::iterator _peakChar;
    std::unordered_map<std::string, Token::TokenType> _identMap;
};


#endif //ZAP_LEXER_H
