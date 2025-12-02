#ifndef ZAP_STRINGLITERAL_H
#define ZAP_STRINGLITERAL_H
#include "Ast.h"
#include "lexer/Token.h"

namespace ast {
    struct StringLiteral : public ExpressionNode {
        std::string String() override;

        std::string TokenLiteral() override;

        inline ~StringLiteral() override;

        std::string value;
        Token::Token token;
    };

    inline std::string StringLiteral::String() {
        return value + " ";
    }

    inline std::string StringLiteral::TokenLiteral() {
        return token.tokenLiteral;
    }

    inline StringLiteral::~StringLiteral() = default;
}

#endif //ZAP_STRINGLITERAL_H