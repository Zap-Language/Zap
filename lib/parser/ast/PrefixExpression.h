#ifndef ZAP_PREFIXEXPRESSION_H
#define ZAP_PREFIXEXPRESSION_H
#include <memory>

#include "Ast.h"
#include "lexer/Token.h"

namespace ast {
    struct PrefixExpression : public ExpressionNode {
        std::string String() override;

        std::string TokenLiteral() override;

        inline ~PrefixExpression() override;

        Token::Token token;
        std::shared_ptr<ExpressionNode> rightExpression;
    };

    inline std::string PrefixExpression::String() {
        return token.tokenLiteral + rightExpression->String();
    }

    inline std::string PrefixExpression::TokenLiteral() {
        return token.tokenLiteral;
    }

    inline PrefixExpression::~PrefixExpression() = default;
}

#endif //ZAP_PREFIXEXPRESSION_H