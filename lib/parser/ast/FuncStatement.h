#ifndef ZAP_FUNCSTATEMENT_H
#define ZAP_FUNCSTATEMENT_H
#include <memory>
#include <vector>

#include "Ast.h"
#include "Identifier.h"
#include "lexer/Token.h"

namespace ast {
    struct FuncStatement final : public ast::StatementNode {
        ~FuncStatement() override = default;

        std::string String() override;

        std::string TokenLiteral() override;

        Token::Token token;
        std::shared_ptr<Identifier> name;
        std::pmr::vector<std::shared_ptr<Identifier>> parameters;
        std::shared_ptr<DataType> returnType;
        std::vector<std::shared_ptr<StatementNode>> body;
    };

    inline std::string FuncStatement::String() {
        std::string result;

        result += "func ";
        result += name->String();
        result += "(";
        for (const auto& parameter : parameters) {
            result += parameter->String();
        }

        result += ") ";
        result += returnType->String();
        result += " {\n";
        for (const auto& stmt : body) {
            result += stmt->String() + '\n';
        }

        result += "}";
        return result;
    }

    inline std::string FuncStatement::TokenLiteral() {
        return token.tokenLiteral;
    }
}

#endif //ZAP_FUNCSTATEMENT_H
