#ifndef ZAP_PARSER_H
#define ZAP_PARSER_H
#include <functional>
#include <lexer/lexer.h>

#include "ast/FuncStatement.h"
#include "ast/LetStatement.h"
#include "ast/Program.h"

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
    private:
        std::shared_ptr<StatementNode> ParseStatement();
        std::shared_ptr<LetStatement> ParseLetStatement();
        std::shared_ptr<FuncStatement> ParseFuncStatement();

        std::shared_ptr<ExpressionNode> ParseExpression(Precedence precedence);
        std::shared_ptr<ExpressionNode> ParsePrefixExpression();
        std::shared_ptr<ExpressionNode> ParseInfixExpression(std::shared_ptr<ExpressionNode> leftExpression);

        std::shared_ptr<DataType> ParseDataType();

        void NextToken();

        bool ExpectPeek(Token::TokenType tokenType);
        bool CurrentTokenIs(Token::TokenType tokenType) const;
        bool PeekTokenIs(Token::TokenType tokenType) const;

        std::shared_ptr<ExpressionNode> ParseIntegerLiteral() const;

        template <typename T>
        void RegisterPrefix(Token::TokenType tokenType, T callback);

        template <typename T>
        void RegisterInfix(Token::TokenType tokenType, T callback);

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

    template<typename T>
    void Parser::RegisterPrefix(Token::TokenType tokenType, T callback) {
        _prefixParseFunction.emplace(tokenType, [callback](){return callback();});
    }

    template<typename T>
    void Parser::RegisterInfix(Token::TokenType tokenType, T callback) {
        _infixParseFunction.emplace(tokenType, [callback](){return callback();});
    }
} // ast

#endif //ZAP_PARSER_H