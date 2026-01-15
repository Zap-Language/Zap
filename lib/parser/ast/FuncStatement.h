#ifndef ZAP_FUNCSTATEMENT_H
#define ZAP_FUNCSTATEMENT_H
#include <memory>
#include <vector>

#include "Ast.h"
#include "Identifier.h"
#include "ArgumentList.h"
#include "BlockStatement.h"
#include "lexer/Token.h"

namespace ast {
    struct FuncStatement final : public ast::StatementNode {
        ~FuncStatement() override = default;

        std::string String() override;

        std::string TokenLiteral() override;

        Token::Token token;
        std::shared_ptr<Identifier> name;
        std::shared_ptr<ArgumentList> arguments;
        std::shared_ptr<DataType> returnType;
        std::shared_ptr<BlockStatement> body;
    };

    inline std::string FuncStatement::String() {
        std::string result;

        result += "func ";
        result += name->String();
        result += "(";
        result += arguments->String();
        result += ") ";
        result += returnType->String();
        result += body->String();
        return result;
    }

    inline std::string FuncStatement::TokenLiteral() {
        return token.tokenLiteral;
    }
}

#endif //ZAP_FUNCSTATEMENT_H
