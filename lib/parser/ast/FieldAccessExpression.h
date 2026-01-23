#ifndef ZAP_FIELDACCESSEXPRESSION_H
#define ZAP_FIELDACCESSEXPRESSION_H

#include <memory>
#include "Ast.h"
#include "Identifier.h"

namespace ast {
    struct FieldAccessExpression final : public ExpressionNode {
        ~FieldAccessExpression() override = default;

        std::string String() override {
            std::string res;
            if (object) res += object->String();
            res += ".";
            if (field) res += field->String();
            return res;
        }

        std::string TokenLiteral() override {
            return ".";
        }

        std::shared_ptr<ExpressionNode> object;
        std::shared_ptr<Identifier> field;
        std::string structName;
        uint32_t fieldIndex = 0;
        bool isMethod = false;
    };
}

#endif // ZAP_FIELDACCESSEXPRESSION_H
