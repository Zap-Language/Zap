#ifndef ZAP_PARSER_H
#define ZAP_PARSER_H
#include <lexer/lexer.h>

#include "ast/FuncStatement.h"
#include "ast/LetStatement.h"
#include "ast/Program.h"

namespace ast {
    class Parser {
    public:
        explicit Parser(Lexer lexer);

        std::unique_ptr<Program> ParseProgram();
    private:
        std::shared_ptr<StatementNode> ParseStatement();
        std::shared_ptr<LetStatement> ParseLetStatement();
        std::shared_ptr<FuncStatement> ParseFuncStatement();

        std::shared_ptr<ExpressionNode> ParseExpression();

        std::shared_ptr<DataType> ParseDataType();

        void NextToken();

        bool ExpectPeek(Token::TokenType tokenType);
        bool CurrentTokenIs(Token::TokenType tokenType) const;
        bool PeekTokenIs(Token::TokenType tokenType) const;

        Lexer _lexer;

        Token::Token _currentToken;
        Token::Token _peekToken;

        std::vector<std::string> _errors;
    };
} // ast

#endif //ZAP_PARSER_H