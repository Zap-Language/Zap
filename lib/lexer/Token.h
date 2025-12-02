#ifndef ZAP_TOKEN_H
#define ZAP_TOKEN_H

#include <string>

namespace Token {
enum TokenType {
        Illegal,
        Eof,

        Ident,
        Integer,
        Float,
        Char,
        String,

        Assign,
        Plus,
        Minus,
        Asterisk,
        Slash,
        Power,
        Percent,

        Bang,
        And,
        Or,

        Equal,
        NotEqual,
        Less,
        Greater,

        Comma,
        Semicolon,
        Colon,
        Dot,
        Apostrophe,
        Quote,
        NewLine,

        LParen,
        RParen,
        LBrace,
        RBrace,
        LBracket,
        RBracket,

        Func,
        Let,
        Return,
        If,
        Else,
        ElseIf,
        For,
        While,
        True,
        False,

        IntType,
        FloatType,
        CharType,
        StringType,
        BoolType,

        Len,
        Read,
        Print,
    };

    struct Token {
        TokenType tokenType;
        std::string tokenLiteral;
    };
}

#endif
