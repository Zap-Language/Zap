#ifndef ZAP_FORSTATEMENT_H
#define ZAP_FORSTATEMENT_H
#include <memory>

#include "Ast.h"
#include "LetStatement.h"
#include "lexer/Token.h"
#include "parser/utils.h"

namespace ast {
    struct ForStatement : public StatementNode {
        std::string String() override;

        std::string TokenLiteral() override;

        inline ~ForStatement() override = default;

        Token::Token token;
        std::shared_ptr<LetStatement> letStatement;
        std::shared_ptr<ExpressionNode> condition;
        std::shared_ptr<ExpressionNode> postExpression;
        std::shared_ptr<StatementNode> stmt;
    };

    inline std::string ForStatement::String() {
        std::string result;
        result += "for (";
        result += trim(letStatement->String());
        result += "; ";
        result += trim(condition->String());
        result += "; ";
        result += trim(postExpression->String());
        result += ") ";
        result += stmt->String();

        return result;
    }

    inline std::string ForStatement::TokenLiteral() {
        return token.tokenLiteral;
    }
}

#endif //ZAP_FORSTATEMENT_H