#ifndef ZAP_FLOATLITERAL_H
#define ZAP_FLOATLITERAL_H
#include "Ast.h"
#include "lexer/Token.h"

namespace ast {
    using float64_t = long double;

    struct FloatLiteral : public ExpressionNode {
        std::string String() override;

        std::string TokenLiteral() override;

        inline ~FloatLiteral() override;

        float64_t value;
        Token::Token token;
    };

    inline std::string FloatLiteral::String() {
        return std::to_string(value) + " ";
    }

    inline std::string FloatLiteral::TokenLiteral() {
        return token.tokenLiteral;
    }

    inline FloatLiteral::~FloatLiteral() = default;
}

#endif //ZAP_FLOATLITERAL_H