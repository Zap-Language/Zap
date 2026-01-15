#ifndef ZAP_RETURNSTATEMENT_H
#define ZAP_RETURNSTATEMENT_H
#include <memory>

#include "Ast.h"
#include "lexer/Token.h"

namespace ast {
    struct ReturnStatement : public StatementNode {
        std::string String() override;

        std::string TokenLiteral() override;

        inline ~ReturnStatement() override;

        Token::Token token;
        std::shared_ptr<ExpressionNode> expression;
    };

    inline std::string ReturnStatement::String() {
        return TokenLiteral() + " " + expression->String();
    }

    inline std::string ReturnStatement::TokenLiteral() {
        return token.tokenLiteral;
    }

    inline ReturnStatement::~ReturnStatement() = default;
}

#endif //ZAP_RETURNSTATEMENT_H