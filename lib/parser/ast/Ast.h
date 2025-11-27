#ifndef ZAP_AST_H
#define ZAP_AST_H
#include <string>

namespace ast {
    struct Node {
        virtual ~Node();

        virtual std::string String() = 0;

        virtual std::string TokenLiteral() = 0;
    };

    struct StatementNode : public Node {
        ~StatementNode() override;
    };

    inline Node::~Node() = default;

    inline StatementNode::~StatementNode() = default;

    struct ExpressionNode : public Node {
        ~ExpressionNode() override;
    };

    inline ExpressionNode::~ExpressionNode() = default;
}
#endif //ZAP_AST_H
