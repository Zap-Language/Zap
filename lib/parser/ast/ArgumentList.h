#ifndef ZAP_ARGUMENTLIST_H
#define ZAP_ARGUMENTLIST_H
#include <memory>
#include <vector>

#include "Ast.h"
#include "Identifier.h"

namespace ast {
    struct ArgumentList : public StatementNode {
        std::string String() override;

        std::string TokenLiteral() override;

        inline ~ArgumentList() override = default;

        std::vector<std::shared_ptr<Identifier>> arguments;
        Token::Token token;
    };

    inline std::string ArgumentList::String() {
        std::string result;

        for (const auto& argument : arguments) {
            result += argument->type->String();
            result += argument->TokenLiteral();
            result += ", ";
        }

        if (!result.empty()) {
            result.pop_back();
            result.pop_back();
        }

        return result;
    }

    inline std::string ArgumentList::TokenLiteral() {
        return token.tokenLiteral;
    }
}

#endif //ZAP_ARGUMENTLIST_H