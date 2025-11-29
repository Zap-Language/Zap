#ifndef ZAP_PROGRAM_H
#define ZAP_PROGRAM_H
#include <memory>
#include <vector>

#include "Ast.h"
#include "lexer/Token.h"

namespace ast {
    struct Program final : public StatementNode {
        ~Program() override = default;

        std::string String() override;

        std::string TokenLiteral() override;

        std::vector<std::shared_ptr<StatementNode>> statements;
        Token::Token _token;
    };

    inline std::string Program::TokenLiteral() {
        return _token.tokenLiteral;
    }

    inline std::string Program::String() {
        std::string out;

        for (const auto &stmt: statements) {
            out += stmt->String() + '\n';
        }

        return out;
    }
}

#endif //ZAP_PROGRAM_H
