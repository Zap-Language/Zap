#ifndef ZAP_IFSTATEMENT_H
#define ZAP_IFSTATEMENT_H
#include <memory>

#include "Ast.h"
#include "lexer/Token.h"
#include "parser/utils.h"

namespace ast {
    struct IfStatement : public StatementNode {
        std::string String() override;

        std::string TokenLiteral() override;

        inline ~IfStatement() override;

        Token::Token token;
        std::shared_ptr<ExpressionNode> condition;
        std::shared_ptr<StatementNode> thenStatement;
        std::shared_ptr<StatementNode> elseStatement;
    };

    inline std::string IfStatement::String() {
        std::string result;

        result += "if (";
        result += trim(condition->String());
        result += ") ";
        result += thenStatement->String();
        if (thenStatement != nullptr) {
            result += thenStatement->String();
        }

        return result;
    }

    inline std::string IfStatement::TokenLiteral() {
        return token.tokenLiteral;
    }

    inline IfStatement::~IfStatement() = default;
}

#endif //ZAP_IFSTATEMENT_H