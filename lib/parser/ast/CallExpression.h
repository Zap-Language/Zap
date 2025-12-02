#ifndef ZAP_CALLEXPRESSION_H
#define ZAP_CALLEXPRESSION_H
#include <memory>
#include <vector>

#include "Ast.h"
#include "lexer/Token.h"
#include "parser/utils.h"

namespace ast {
    struct CallExpression : public ExpressionNode {
        std::string String() override;

        std::string TokenLiteral() override;

        inline ~CallExpression() override;

        Token::Token token;
        std::shared_ptr<ExpressionNode> function;
        std::vector<std::shared_ptr<ExpressionNode>> arguments;
    };

    inline std::string CallExpression::String() {
        std::string result;

        result += trim(function->String());
        result += "(";
        for (const auto& arg : arguments) {
            result += trim(arg->String());
            result += ", ";
        }

        if (!result.empty()) {
            result.pop_back();
            result.pop_back();
        }

        result += ") ";
        return result;
    }

    inline std::string CallExpression::TokenLiteral() {
        return token.tokenLiteral;
    }

    inline CallExpression::~CallExpression() = default;
}

#endif //ZAP_CALLEXPRESSION_H