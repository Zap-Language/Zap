#ifndef ZAP_LEXER_H
#define ZAP_LEXER_H


#include <string>
#include "Token.h"

class Lexer {
public:
    explicit Lexer(std::string& input) {
        std::copy(input.begin(), input.end(), _input.begin());
        _peakChar = input.begin();
        this->ReadChar();
    }

    Token::Token NextToken();

private:
    void SkipWhitespaces();

    std::string ReadIdent();

    std::string ReadNumber();

    void ReadChar() {
        _curChar = _peakChar;
        _peakChar++;
    }

    std::string _input;
    std::string::iterator _curChar;
    std::string::iterator _peakChar;
};


#endif //ZAP_LEXER_H
