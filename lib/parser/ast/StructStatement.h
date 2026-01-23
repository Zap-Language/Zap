#ifndef ZAP_STRUCTSTATEMENT_H
#define ZAP_STRUCTSTATEMENT_H

#include <memory>
#include <string>
#include <vector>
#include "Ast.h"
#include "Identifier.h"
#include "FuncStatement.h"
#include "DataTypeStruct.h"

namespace ast {
    struct StructField {
        Token::Token token;
        std::string name;
        std::shared_ptr<DataType> type;
    };

    struct StructStatement final : public StatementNode, public std::enable_shared_from_this<StructStatement> {
        ~StructStatement() override = default;

        std::string String() override {
            std::string res = "struct " + name + " {";
            for (const auto& f : fields) {
                res += f.name + ": " + (f.type ? f.type->String() : "unknown") + "; ";
            }
            res += "}";
            return res;
        }

        std::string TokenLiteral() override { return token.tokenLiteral; }

        Token::Token token;
        std::string name;
        std::vector<StructField> fields;
        std::vector<std::shared_ptr<FuncStatement>> methods;
    };
}

#endif // ZAP_STRUCTSTATEMENT_H
