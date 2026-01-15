#ifndef ZAP_LETSTATEMENT_H
#define ZAP_LETSTATEMENT_H
#include <memory>

#include "Ast.h"
#include "Identifier.h"
#include "lexer/Token.h"

namespace ast {
    struct LetStatement final : public ast::StatementNode {
        ~LetStatement() override = default;

        std::string String() override;

        std::string TokenLiteral() override;

        Token::Token token;
        std::shared_ptr<Identifier> name;
        std::shared_ptr<ExpressionNode> value;
    };

    inline std::string LetStatement::String() {
        std::string result;

        result += "let ";
        result += name->String();
        result += "= ";
        result += value->String();

        return result;
    }

    inline std::string LetStatement::TokenLiteral() {
        return token.tokenLiteral;
    }
}

#endif //ZAP_LETSTATEMENT_H
