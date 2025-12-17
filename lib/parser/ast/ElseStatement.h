#ifndef ZAP_ELSESTATEMENT_H
#define ZAP_ELSESTATEMENT_H
#include <memory>

#include "Ast.h"
#include "lexer/Token.h"

namespace ast {
    struct ElseStatement : public StatementNode {
        std::string String() override;

        std::string TokenLiteral() override;

        ~ElseStatement() override = default;

        Token::Token token;
        std::shared_ptr<StatementNode> stmt;
    };

    inline std::string ElseStatement::String() {
        std::string result;

        result += "else ";
        result += stmt->String();
        return result;
    }

    inline std::string ElseStatement::TokenLiteral() {
        return token.tokenLiteral;
    }
}

#endif //ZAP_ELSESTATEMENT_H