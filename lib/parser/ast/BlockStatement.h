#ifndef ZAP_BLOCKSTATEMENT_H
#define ZAP_BLOCKSTATEMENT_H
#include <memory>
#include <vector>

#include "Ast.h"
#include "lexer/Token.h"

namespace ast {
    const std::string LEFT_MARGIN = "    ";
    // const std::string LEFT_MARGIN = std::string(" ", 4);

    struct BlockStatement : public StatementNode {
        std::string String() override;

        std::string TokenLiteral() override;

        inline ~BlockStatement() override;

        std::vector<std::shared_ptr<StatementNode>> statements;
        Token::Token token;
    };

    inline std::string BlockStatement::String() {
        std::string result;

        result += "{\n";
        for (const auto& item : statements) {
            result += LEFT_MARGIN + item->String() + '\n';
        }

        result += "}";

        return result;
    }

    inline std::string BlockStatement::TokenLiteral() {
        return token.tokenLiteral;
    }

    inline BlockStatement::~BlockStatement() = default;
}

#endif //ZAP_BLOCKSTATEMENT_H