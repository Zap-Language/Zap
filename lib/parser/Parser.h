#ifndef ZAP_PARSER_H
#define ZAP_PARSER_H
#include <functional>
#include <lexer/lexer.h>

#include "ast/ArgumentList.h"
#include "ast/AssignStatement.h"
#include "ast/BlockStatement.h"
#include "ast/BoolLiteral.h"
#include "ast/CallExpression.h"
#include "ast/CharLiteral.h"
#include "ast/ElseStatement.h"
#include "ast/ExpressionStatement.h"
#include "ast/FloatLiteral.h"
#include "ast/ForStatement.h"
#include "ast/FuncExpression.h"
#include "ast/FuncStatement.h"
#include "ast/IfStatement.h"
#include "ast/IndexExpression.h"
#include "ast/IntLiteral.h"
#include "ast/LetStatement.h"
#include "ast/Program.h"
#include "ast/ReturnStatement.h"
#include "ast/StringLiteral.h"
#include "ast/ArrayLiteral.h"
#include "ast/WhileStatement.h"

namespace ast {
    enum Precedence {
        LOWEST,
        OR,
        AND,
        EQUALS,
        LESSGREATER,
        SUM,
        PRODUCT,
        PREFIX,
        CALL,
        INDEX,
    };

    class Parser {
    public:
        explicit Parser(Lexer lexer);

        std::unique_ptr<Program> ParseProgram();

        void PrintErrors() const;
        std::shared_ptr<StatementNode> ParseStatement();
        std::shared_ptr<LetStatement> ParseLetStatement();
        std::shared_ptr<FuncStatement> ParseFuncStatement();
        std::shared_ptr<BlockStatement> ParseBlockStatement();
        std::shared_ptr<ReturnStatement> ParseReturnStatement();
        std::shared_ptr<IfStatement> ParseIfStatement();
        std::shared_ptr<ElseStatement> ParseElseStatement();
        std::shared_ptr<AssignStatement> ParseAssignStatement();
        std::shared_ptr<ExpressionStatement> ParseExpressionStatement();

        std::shared_ptr<WhileStatement> ParseWhileStatement();

        std::shared_ptr<ForStatement> ParseForStatement();

        std::shared_ptr<ArgumentList> ParseArgumentList();

        std::shared_ptr<Identifier> ParseIdentifier(std::shared_ptr<DataType> dataType) const;
        std::shared_ptr<ExpressionNode> ParseExpression(Precedence precedence);
        std::shared_ptr<ExpressionNode> ParsePrefixExpression();
        std::shared_ptr<ExpressionNode> ParseInfixExpression(std::shared_ptr<ExpressionNode> leftExpression);
        std::shared_ptr<FuncExpression> ParseFuncExpression();
        std::shared_ptr<CallExpression> ParseCallExpression(std::shared_ptr<ExpressionNode> leftExpression);
        std::shared_ptr<IndexExpression> ParseIndexExpression(std::shared_ptr<ExpressionNode> leftExpression);
        std::shared_ptr<ArrayLiteral> ParseArrayLiteral();
        std::vector<std::shared_ptr<ExpressionNode>> ParseCallArguments();

        std::shared_ptr<DataType> ParseDataType();

        Precedence PeekPrecedence() const;

        void NextToken();

        bool ExpectPeek(Token::TokenType tokenType);
        bool CurrentTokenIs(Token::TokenType tokenType) const;
        bool PeekTokenIs(Token::TokenType tokenType) const;

        std::shared_ptr<IntLiteral> ParseIntegerLiteral() const;
        std::shared_ptr<FloatLiteral> ParseFloatLiteral() const;
        std::shared_ptr<BoolLiteral> ParseBoolLiteral() const;
        std::shared_ptr<CharLiteral> ParseCharLiteral();
        std::shared_ptr<StringLiteral> ParseStringLiteral();

        Lexer _lexer;

        Token::Token _currentToken;
        Token::Token _peekToken;

        std::vector<std::string> _errors;

        std::unordered_map<
            Token::TokenType,
            std::function<std::shared_ptr<ExpressionNode>()>
        > _prefixParseFunction;

        std::unordered_map<
            Token::TokenType,
            std::function<std::shared_ptr<ExpressionNode>(std::shared_ptr<ExpressionNode>)>
        > _infixParseFunction;

        std::unordered_map<Token::TokenType, Precedence> _precedence;
    };
} // ast

#endif //ZAP_PARSER_H