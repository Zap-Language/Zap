#ifndef ZAP_NEWEXPRESSION_H
#define ZAP_NEWEXPRESSION_H

#include <memory>
#include <vector>
#include "Ast.h"
#include "Identifier.h"
#include "DataTypeStruct.h"

namespace ast {
    struct NewExpression final : public ExpressionNode {
        ~NewExpression() override = default;

        std::string String() override {
            std::string res = "new ";
            if (type) res += type->String();
            res += "(";
            for (size_t i = 0; i < arguments.size(); ++i) {
                if (i) res += ", ";
                res += arguments[i]->String();
            }
            res += ")";
            return res;
        }

        std::string TokenLiteral() override { return "new"; }

        std::shared_ptr<DataTypeStruct> type;
        std::vector<std::shared_ptr<ExpressionNode>> arguments;
    };
}

#endif // ZAP_NEWEXPRESSION_H
