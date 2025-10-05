#ifndef ZAP_TOKEN_H
#define ZAP_TOKEN_H

#include <string>

namespace Token {
    enum TokenType {
        Ident,
        Eof,
        Assign,
        Greater,
        Less,
        Illegal,
        Integer,
    };

    struct Token {
        TokenType tokenType;
        std::string tokenLiteral;
    };
}

#endif //ZAP_TOKEN_H
