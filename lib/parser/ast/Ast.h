#ifndef ZAP_AST_H
#define ZAP_AST_H
#include <string>

namespace ast {
    struct Node {
        virtual ~Node() = 0;

        virtual std::string String() = 0;

        virtual std::string TokenLiteral() = 0;
    };

    struct StatementNode : public Node {
    };

    struct ExpressionNode : public Node {
    };
}
#endif //ZAP_AST_H
