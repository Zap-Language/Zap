#include "Parser.h"

#include "ast/DataTypeInt.h"
#include "ast/IntLiteral.h"

namespace ast {
    Parser::Parser(Lexer lexer) : _lexer(std::move(lexer)) {
        _prefixParseFunction.emplace(Token::Integer, [this](){return this->ParseIntegerLiteral();});

        _precedence.emplace(Token::Or, OR);
        _precedence.emplace(Token::And, AND);
        _precedence.emplace(Token::Equal, EQUALS);
        _precedence.emplace(Token::NotEqual, EQUALS);
        _precedence.emplace(Token::Less, LESSGREATER);
        _precedence.emplace(Token::Greater, LESSGREATER);
        _precedence.emplace(Token::Plus, SUM);
        _precedence.emplace(Token::Minus, SUM);
        _precedence.emplace(Token::Asterisk, PRODUCT);
        _precedence.emplace(Token::Slash, PRODUCT);
        _precedence.emplace(Token::LParen, CALL);
        _precedence.emplace(Token::LBracket, INDEX);

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
        std::shared_ptr<ExpressionNode> expression = ParseExpression(LOWEST);
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
           _errors.emplace_back("peek expected " + _peekToken.tokenLiteral);
            return false;
        }

        NextToken();
        return true;
    }

    std::shared_ptr<FuncStatement> Parser::ParseFuncStatement() {
        return nullptr;
    }

    std::shared_ptr<ExpressionNode> Parser::ParseExpression(Precedence precedence) {
        if (!_prefixParseFunction.contains(_currentToken.tokenType)) {
            _errors.emplace_back("function expected for " + _currentToken.tokenLiteral);
            return nullptr;
        }

        auto prefix = _prefixParseFunction.at(_currentToken.tokenType);
        std::shared_ptr<ExpressionNode> leftExpression = prefix();
        while (!PeekTokenIs(Token::NewLine) && !PeekTokenIs(Token::Eof) && precedence < _precedence.at(_peekToken.tokenType)) {

        }

        return leftExpression;
    }

    bool Parser::CurrentTokenIs(const Token::TokenType tokenType) const {
        return _currentToken.tokenType == tokenType;
    }

    bool Parser::PeekTokenIs(const Token::TokenType tokenType) const {
        return _peekToken.tokenType == tokenType;
    }

    std::shared_ptr<ExpressionNode> Parser::ParseIntegerLiteral() {
        auto integerLiteral = std::make_shared<IntLiteral>();
        integerLiteral->token = _currentToken;
        integerLiteral->value = std::strtoll(_currentToken.tokenLiteral.c_str(), nullptr, 10);

        return integerLiteral;
    }
} // ast