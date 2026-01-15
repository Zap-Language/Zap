#ifndef ZAP_CHARLITERAL_H
#define ZAP_CHARLITERAL_H
#include "Ast.h"
#include "lexer/Token.h"

namespace ast {
    struct CharLiteral : public ExpressionNode {
        std::string String() override;

        std::string TokenLiteral() override;

        inline ~CharLiteral() override;

        char value;
        Token::Token token;
    };

    inline std::string CharLiteral::String() {
        return std::to_string(value) + " ";
    }

    inline std::string CharLiteral::TokenLiteral() {
        return token.tokenLiteral;
    }

    inline CharLiteral::~CharLiteral() = default;
}

#endif //ZAP_CHARLITERAL_H