#ifndef ZAP_INFIXEXPRESSION_H
#define ZAP_INFIXEXPRESSION_H
#include <memory>

#include "Ast.h"
#include "lexer/Token.h"

namespace ast {
    struct InfixExpression : public ExpressionNode {
        std::string String() override;

        std::string TokenLiteral() override;

        ~InfixExpression() override = default;

        Token::Token token;
        std::shared_ptr<ExpressionNode> leftExpression;
        std::shared_ptr<ExpressionNode> rightExpression;
    };

    inline std::string InfixExpression::String() {
        return leftExpression->String() + token.tokenLiteral + rightExpression->String();
    }

    inline std::string InfixExpression::TokenLiteral() {
        return token.tokenLiteral;
    }
}

#endif //ZAP_INFIXEXPRESSION_H