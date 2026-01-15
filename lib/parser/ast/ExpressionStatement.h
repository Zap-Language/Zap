#ifndef ZAP_EXPRESSIONSTATEMENT_H
#define ZAP_EXPRESSIONSTATEMENT_H
#include <memory>

#include "Ast.h"
#include "lexer/Token.h"

namespace ast {
    struct ExpressionStatement : public StatementNode {
        std::string String() override;

        std::string TokenLiteral() override;

        inline ~ExpressionStatement() override = default;

        Token::Token token;
        std::shared_ptr<ExpressionNode> expression;
    };

    inline std::string ExpressionStatement::String() {
        return expression->String();
    }

    inline std::string ExpressionStatement::TokenLiteral() {
        return token.tokenLiteral;
    }
}

#endif //ZAP_EXPRESSIONSTATEMENT_H