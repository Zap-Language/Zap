#pragma once
#include <memory>
#include "Ast.h"

namespace ast {
    struct IndexExpression final : public ExpressionNode {
        ~IndexExpression() override = default;

        std::string String() override;
        std::string TokenLiteral() override;

        Token::Token token; // '['
        std::shared_ptr<ExpressionNode> left;
        std::shared_ptr<ExpressionNode> index;
    };

    inline std::string IndexExpression::String() {
        return "(" + (left ? left->String() : "") + "[" + (index ? index->String() : "") + "])";
    }

    inline std::string IndexExpression::TokenLiteral() {
        return token.tokenLiteral;
    }
}