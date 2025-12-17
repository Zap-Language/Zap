#ifndef ZAP_WHILESTATEMENT_H
#define ZAP_WHILESTATEMENT_H
#include <memory>

#include "Ast.h"
#include "lexer/Token.h"
#include "parser/utils.h"

namespace ast {
    struct WhileStatement : public StatementNode {
        std::string String() override;

        std::string TokenLiteral() override;

        ~WhileStatement() override = default;

        Token::Token token;
        std::shared_ptr<ExpressionNode> condition;
        std::shared_ptr<StatementNode> stmt;
    };

    inline std::string WhileStatement::String() {
        std::string result;
        result += "while (";
        result += trim(condition->String());
        result += ")";
        result += stmt->String();
        return result;
    }

    inline std::string WhileStatement::TokenLiteral() {
        return token.tokenLiteral;
    }
}

#endif //ZAP_WHILESTATEMENT_H