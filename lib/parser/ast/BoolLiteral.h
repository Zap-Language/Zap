#ifndef ZAP_BOOLLITERAL_H
#define ZAP_BOOLLITERAL_H
#include "Ast.h"
#include "lexer/Token.h"

namespace ast {
    struct BoolLiteral : public ExpressionNode {
        std::string String() override;

        std::string TokenLiteral() override;

        inline ~BoolLiteral() override;

        bool value;
        Token::Token token;
    };

    inline std::string BoolLiteral::String() {
        return std::to_string(value) + " ";
    }

    inline std::string BoolLiteral::TokenLiteral() {
        return token.tokenLiteral;
    }

    inline BoolLiteral::~BoolLiteral() = default;
}

#endif //ZAP_BOOLLITERAL_H