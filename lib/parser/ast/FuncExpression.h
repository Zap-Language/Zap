#ifndef ZAP_FUNCEXPRESSION_H
#define ZAP_FUNCEXPRESSION_H
#include <memory>

#include "ArgumentList.h"
#include "Ast.h"
#include "BlockStatement.h"
#include "lexer/Token.h"

namespace ast {
    struct FuncExpression final : public ast::ExpressionNode {
        ~FuncExpression() override = default;

        std::string String() override;

        std::string TokenLiteral() override;

        Token::Token token;
        std::shared_ptr<ArgumentList> arguments;
        std::shared_ptr<DataType> returnType;
        std::shared_ptr<BlockStatement> body;
    };

    inline std::string FuncExpression::String() {
        std::string result;

        result += "func";
        result += "(";
        result += arguments->String();
        result += ") ";
        result += returnType->String();
        result += body->String();
        return result;
    }

    inline std::string FuncExpression::TokenLiteral() {
        return token.tokenLiteral;
    }
}

#endif //ZAP_FUNCEXPRESSION_H
