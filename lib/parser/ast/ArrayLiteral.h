#ifndef ZAP_ARRAYLITERAL_H
#define ZAP_ARRAYLITERAL_H

#include <memory>
#include <vector>

#include "Ast.h"
#include "DataType.h"

namespace ast {
    struct ArrayLiteral final : public ExpressionNode {
        ~ArrayLiteral() override = default;

        std::string String() override;
        std::string TokenLiteral() override;

        Token::Token token;
        std::vector<std::shared_ptr<ExpressionNode>> elements;
        std::shared_ptr<DataType> elementType;
    };

    inline std::string ArrayLiteral::String() {
        std::string out = "[";
        for (size_t i = 0; i < elements.size(); ++i) {
            out += elements[i] ? elements[i]->String() : "";
            if (i + 1 < elements.size()) out += ", ";
        }
        out += "]";
        return out;
    }

    inline std::string ArrayLiteral::TokenLiteral() {
        return token.tokenLiteral;
    }
}

#endif //ZAP_ARRAYLITERAL_H

