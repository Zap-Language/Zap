#ifndef ZAP_TOKEN_H
#define ZAP_TOKEN_H

#include <string>

namespace Token {
    typedef std::string TokenType;

    struct Token {
        TokenType tokenType;
        std::string tokenLiteral;
    };


    const TokenType Ident = "IDENT";
    const TokenType Eof = "EOF";
    const TokenType Assign = ":=";
    const TokenType Greater = ">";
    const TokenType Less = "<";
    const TokenType Illegal = "ILLEGAL";
    const TokenType Integer = "INTEGER";
}

#endif //ZAP_TOKEN_H
