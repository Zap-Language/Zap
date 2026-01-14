#ifndef ZAP_ASSIGNSTATEMENT_H
#define ZAP_ASSIGNSTATEMENT_H
#include <memory>

#include "Ast.h"
#include "Identifier.h"
#include "IndexExpression.h"
#include "lexer/Token.h"

namespace ast {
    struct AssignStatement : public StatementNode {
        std::string String() override;

        std::string TokenLiteral() override;

        inline ~AssignStatement() override = default;

        Token::Token token;
        std::shared_ptr<ExpressionNode> target;
        std::shared_ptr<ExpressionNode> expression;
    };

    inline std::string AssignStatement::String() {
        std::string result;

        if (target) {
            result += target->String();
            result += "= ";
        }
        if (expression) {
            result += expression->String();
        }

        return result;
    }

    inline std::string AssignStatement::TokenLiteral() {
        return token.tokenLiteral;
    }
}

#endif //ZAP_ASSIGNSTATEMENT_H