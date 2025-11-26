#include "Parser.h"

#include <format>
#include <utility>

#include "ast/DataTypeInt.h"

namespace ast {
    Parser::Parser(Lexer lexer) : _lexer(std::move(lexer)) {
        NextToken();
        NextToken();
    }

    std::unique_ptr<Program> Parser::ParseProgram() {
        auto program = std::make_unique<Program>();

        while (!CurrentTokenIs(Token::Eof)) {
            auto statement = ParseStatement();
            if (statement != nullptr) {
                program->statements.emplace_back(statement);
            }

            NextToken();
        }

        return program;
    }

    std::shared_ptr<StatementNode> Parser::ParseStatement() {
        switch (_currentToken.tokenType) {
            case Token::Func:
                return ParseFuncStatement();
            case Token::Let:
                return ParseLetStatement();
            default:
                _errors.emplace_back("unknown token type");
                return nullptr;
        }
    }

    std::shared_ptr<LetStatement> Parser::ParseLetStatement() {
        LetStatement statement;
        statement.token = _currentToken;

        if (!ExpectPeek(Token::Ident)) {
            return nullptr;
        }

        auto name = std::make_shared<Identifier>();
        name->token = _currentToken;

        if (!ExpectPeek(Token::Assign)) {
            return nullptr;
        }

        NextToken();
        std::shared_ptr<ExpressionNode> expression = ParseExpression();
        if (expression == nullptr) {
            return nullptr;
        }

        statement.name = name;
        statement.value = expression;

        return std::make_shared<LetStatement>(statement);
    }

    std::shared_ptr<DataType> Parser::ParseDataType() {
        switch (_currentToken.tokenType) {
            case Token::IntType:
                return std::make_shared<DataTypeInt>();
            default:
                _errors.emplace_back("unknown data type: " + _currentToken.tokenLiteral);
                return nullptr;
        }
    }

    void Parser::NextToken() {
        _currentToken = _peekToken;
        _peekToken = _lexer.NextToken();
    }

    bool Parser::ExpectPeek(Token::TokenType tokenType) {
        if (_peekToken.tokenType != tokenType) {
           _errors.emplace_back("peek expected " + std::string(typeid(tokenType).name()));
            return false;
        }

        NextToken();
        return true;
    }

    bool Parser::CurrentTokenIs(const Token::TokenType tokenType) const {
        return _currentToken.tokenType == tokenType;
    }

    bool Parser::PeekTokenIs(const Token::TokenType tokenType) const {
        return _peekToken.tokenType == tokenType;
    }
} // ast