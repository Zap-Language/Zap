#ifndef ZAP_IDENTIFIER_H
#define ZAP_IDENTIFIER_H
#include <memory>

#include "Ast.h"
#include "DataType.h"
#include "lexer/Token.h"

namespace ast {
    struct Identifier : public ExpressionNode {
        std::string String() override;

        std::string TokenLiteral() override;

        Token::Token token;
        std::shared_ptr<DataType> type;
    };

    inline std::string Identifier::String() {
        return token.tokenLiteral;
    }

    inline std::string Identifier::TokenLiteral() {
        return token.tokenLiteral;
    }
}

#endif //ZAP_IDENTIFIER_H