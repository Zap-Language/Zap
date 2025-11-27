#ifndef ZAP_INTLITERAL_H
#define ZAP_INTLITERAL_H
#include <cstdint>

#include "Ast.h"
#include "DataType.h"
#include "lexer/Token.h"

namespace ast {
    struct IntLiteral final : public ExpressionNode {
        std::string String() override;

        std::string TokenLiteral() override;

        Token::Token token;
        int64_t value;
    };

    inline std::string IntLiteral::String() {
        return std::to_string(value);
    }

    inline std::string IntLiteral::TokenLiteral() {
        return token.tokenLiteral;
    }
}

#endif //ZAP_INTLITERAL_H