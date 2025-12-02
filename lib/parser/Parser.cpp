#include "Parser.h"

#include <magic_enum/magic_enum.hpp>
#include <algorithm>
#include <iostream>
#include <utility>

#include "ast/DataTypeBool.h"
#include "ast/DataTypeFloat.h"
#include "ast/DataTypeFunc.h"
#include "ast/DataTypeInt.h"
#include "ast/DataTypeString.h"
#include "ast/DataTypeVoid.h"
#include "ast/InfixExpression.h"
#include "ast/IntLiteral.h"
#include "ast/PrefixExpression.h"

namespace ast {
    Parser::Parser(Lexer lexer) : _lexer(std::move(lexer)) {
        // Prefix parsers preparation
        _prefixParseFunction.emplace(Token::Integer, [this] { return ParseIntegerLiteral(); });
        _prefixParseFunction.emplace(Token::Ident, [this] { return ParseIdentifier(nullptr); });
        _prefixParseFunction.emplace(Token::Float, [this] { return ParseFloatLiteral(); });
        _prefixParseFunction.emplace(Token::True, [this] { return ParseBoolLiteral(); });
        _prefixParseFunction.emplace(Token::False, [this] { return ParseBoolLiteral(); });
        _prefixParseFunction.emplace(Token::Apostrophe, [this] { return ParseCharLiteral(); });
        _prefixParseFunction.emplace(Token::Quote, [this] { return ParseStringLiteral();});
        _prefixParseFunction.emplace(Token::Bang, [this] { return ParsePrefixExpression(); });
        _prefixParseFunction.emplace(Token::Minus, [this] { return ParsePrefixExpression(); });
        _prefixParseFunction.emplace(Token::Func, [this] { return ParseFuncExpression(); });

        // Infix parsers preparation
        _infixParseFunction.emplace(Token::Plus, [this] (const std::shared_ptr<ExpressionNode> &rightExpression) { return ParseInfixExpression(rightExpression); });
        _infixParseFunction.emplace(Token::Minus, [this] (const std::shared_ptr<ExpressionNode> &rightExpression) { return ParseInfixExpression(rightExpression); });
        _infixParseFunction.emplace(Token::Asterisk, [this] (const std::shared_ptr<ExpressionNode> &rightExpression) { return ParseInfixExpression(rightExpression); });
        _infixParseFunction.emplace(Token::Slash, [this] (const std::shared_ptr<ExpressionNode> &rightExpression) { return ParseInfixExpression(rightExpression); });
        _infixParseFunction.emplace(Token::Percent, [this] (const std::shared_ptr<ExpressionNode> &rightExpression) { return ParseInfixExpression(rightExpression); });
        _infixParseFunction.emplace(Token::Or, [this] (const std::shared_ptr<ExpressionNode> &rightExpression) { return ParseInfixExpression(rightExpression); });
        _infixParseFunction.emplace(Token::And, [this] (const std::shared_ptr<ExpressionNode> &rightExpression) { return ParseInfixExpression(rightExpression); });
        _infixParseFunction.emplace(Token::Equal, [this] (const std::shared_ptr<ExpressionNode> &rightExpression) { return ParseInfixExpression(rightExpression); });
        _infixParseFunction.emplace(Token::NotEqual, [this] (const std::shared_ptr<ExpressionNode> &rightExpression) { return ParseInfixExpression(rightExpression); });
        _infixParseFunction.emplace(Token::Less, [this] (const std::shared_ptr<ExpressionNode> &rightExpression) { return ParseInfixExpression(rightExpression); });
        _infixParseFunction.emplace(Token::Greater, [this] (const std::shared_ptr<ExpressionNode> &rightExpression) { return ParseInfixExpression(rightExpression); });

        _precedence.emplace(Token::Or, OR);
        _precedence.emplace(Token::And, AND);
        _precedence.emplace(Token::Equal, EQUALS);
        _precedence.emplace(Token::NotEqual, EQUALS);
        _precedence.emplace(Token::Less, LESSGREATER);
        _precedence.emplace(Token::Greater, LESSGREATER);
        _precedence.emplace(Token::Plus, SUM);
        _precedence.emplace(Token::Minus, SUM);
        _precedence.emplace(Token::Percent, SUM);
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

    void Parser::PrintErrors() const {
        if (!_errors.empty()) {
            std::cout << std::string(8, '=') << "ERRORS:" << std::string(8, '=') << std::endl;
        }

        for (const auto& error : _errors) {
            std::cout << error << std::endl;
        }


        if (!_errors.empty()) {
            std::cout << std::string(23, '=') << std::endl;
        }
    }

    std::shared_ptr<StatementNode> Parser::ParseStatement() {
        switch (_currentToken.tokenType) {
            case Token::Func:
                return ParseFuncStatement();
            case Token::Let:
                return ParseLetStatement();
            case Token::NewLine:
                return nullptr;
            default:
                auto tokenName =  magic_enum::enum_name(_currentToken.tokenType);
                _errors.emplace_back("unknown token type: " + std::string(tokenName));
                return nullptr;
        }
    }

    std::shared_ptr<LetStatement> Parser::ParseLetStatement() {
        LetStatement statement;
        statement.token = _currentToken;

        if (!ExpectPeek(Token::Ident)) {
            return nullptr;
        }

        const auto name = std::make_shared<Identifier>();
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
                return ast::INT;
            case Token::FloatType:
                return ast::FLOAT;
            case Token::StringType:
                return ast::STRING;
            case Token::BoolType:
                return ast::BOOL;
            case Token::Char:
                return ast::CHAR;
            case Token::LBracket: {
                if (!ExpectPeek(Token::RBracket)) {
                    return nullptr;
                }

                NextToken();
                return std::make_shared<DataTypeArray>(ast::DataTypeArray(ParseDataType()));
            }
            default:
                _errors.emplace_back("unknown data type: " + _currentToken.tokenLiteral);
                return nullptr;
        }
    }

    void Parser::NextToken() {
        _currentToken = _peekToken;
        _peekToken = _lexer.NextToken();
    }

    bool Parser::ExpectPeek(const Token::TokenType tokenType) {
        if (_peekToken.tokenType != tokenType) {
            auto tokenName =  magic_enum::enum_name(tokenType);
            auto peekTokenName =  magic_enum::enum_name(_peekToken.tokenType);
           _errors.emplace_back("peek expected " + std::string(tokenName) + ", got: " + std::string(peekTokenName));
            return false;
        }

        NextToken();
        return true;
    }

    std::shared_ptr<FuncStatement> Parser::ParseFuncStatement() {
        auto funcDataType = std::make_shared<DataTypeFunc>();
        auto funcStatement = std::make_shared<FuncStatement>();
        funcStatement->token = _currentToken;
        if (!ExpectPeek(Token::Ident)) {
            return nullptr;
        }

        funcStatement->name = ParseIdentifier(funcDataType);
        if (!ExpectPeek(Token::LParen)) {
            return nullptr;
        }

        funcStatement->arguments = ParseArgumentList();
        if (const auto args = funcStatement->arguments; args != nullptr) {
            funcDataType->params.reserve(args->arguments.size());
            std::ranges::transform(args->arguments, std::back_inserter(funcDataType->params), &Identifier::type);
        } else {
            return nullptr;
        }

        NextToken();
        std::shared_ptr<DataType> returnType;
        if (!CurrentTokenIs(Token::LBrace)) {
            returnType = ParseDataType();
            if (returnType == nullptr) {
                return nullptr;
            }

            NextToken();
        } else {
            returnType = VOID;
        }

        funcStatement->returnType = returnType;
        funcDataType->returnType = returnType;

        if (!CurrentTokenIs(Token::LBrace)) {
            _errors.emplace_back("function expected '{' literal, got: " + _currentToken.tokenLiteral);
            return nullptr;
        }

        const auto body = ParseBlockStatement();
        if (body == nullptr) {
            return nullptr;
        }

        funcStatement->body = body;
        return funcStatement;
    }

    std::shared_ptr<BlockStatement> Parser::ParseBlockStatement() {
        auto blockStatement = std::make_shared<BlockStatement>();
        if (!CurrentTokenIs(Token::LBrace)) {
            _errors.emplace_back("expected '{' for block statement " + _currentToken.tokenLiteral);
            return nullptr;
        }

        NextToken();
        while (!CurrentTokenIs(Token::RBrace) && !CurrentTokenIs(Token::Eof)) {
            auto statement = ParseStatement();
            if (statement != nullptr) {
                blockStatement->statements.emplace_back(statement);
            }

            NextToken();
        }

        return blockStatement;
    }

    std::shared_ptr<ArgumentList> Parser::ParseArgumentList() {
        auto argumentList = std::make_shared<ArgumentList>();
        argumentList->token = _currentToken;
        NextToken();
        while (!CurrentTokenIs(Token::RParen)) {
            auto dataType = ParseDataType();
            if (dataType == nullptr) {
                return nullptr;
            }

            if (!ExpectPeek(Token::Ident)) {
                return nullptr;
            }

            auto param = ParseIdentifier(dataType);
            NextToken();
            if (CurrentTokenIs(Token::Comma)) {
                NextToken();
            }

            argumentList->arguments.emplace_back(param);
        }

        return argumentList;
    }

    std::shared_ptr<Identifier> Parser::ParseIdentifier(std::shared_ptr<DataType> dataType) const {
        auto identifier = std::make_shared<Identifier>();
        identifier->token = _currentToken;
        identifier->type = std::move(dataType);

        return identifier;
    }

    std::shared_ptr<ExpressionNode> Parser::ParseExpression(const Precedence precedence) {
        if (!_prefixParseFunction.contains(_currentToken.tokenType)) {
            _errors.emplace_back("function expected for " + _currentToken.tokenLiteral);
            return nullptr;
        }

        auto prefix = _prefixParseFunction.at(_currentToken.tokenType);
        std::shared_ptr<ExpressionNode> leftExpression = prefix();
        while (!PeekTokenIs(Token::NewLine) && !PeekTokenIs(Token::Eof) && precedence < _precedence.at(_peekToken.tokenType)) {
            auto infix = _infixParseFunction.at(_peekToken.tokenType);
            if (infix == nullptr) {
                return leftExpression;
            }

            NextToken();
            leftExpression = infix(leftExpression);
        }

        return leftExpression;
    }

    std::shared_ptr<ExpressionNode> Parser::ParsePrefixExpression() {
        auto expression = std::make_shared<PrefixExpression>();
        expression->token = _currentToken;

        NextToken();
        expression->rightExpression = ParseExpression(PREFIX);

        return expression;
    }

    std::shared_ptr<ExpressionNode> Parser::ParseInfixExpression(std::shared_ptr<ExpressionNode> leftExpression) {
        auto expression = std::make_shared<InfixExpression>();
        expression->leftExpression = std::move(leftExpression);
        expression->token = _currentToken;

        const auto precedence = _precedence.at(_currentToken.tokenType);
        NextToken();
        expression->rightExpression = ParseExpression(precedence);

        return expression;
    }

    std::shared_ptr<FuncExpression> Parser::ParseFuncExpression() {
        // TODO: надо от повторений c Parser::ParseFuncStatement() избавиться
        auto funcExpression = std::make_shared<FuncExpression>();
        funcExpression->token = _currentToken;
        if (!ExpectPeek(Token::LParen)) {
            return nullptr;
        }

        funcExpression->arguments = ParseArgumentList();
        if (funcExpression->arguments == nullptr) {
            return nullptr;
        }

        NextToken();
        std::shared_ptr<DataType> returnType;
        if (!CurrentTokenIs(Token::LBrace)) {
            returnType = ParseDataType();
            if (returnType == nullptr) {
                return nullptr;
            }

            NextToken();
        } else {
            returnType = VOID;
        }

        funcExpression->returnType = returnType;

        if (!CurrentTokenIs(Token::LBrace)) {
            _errors.emplace_back("function expected '{' literal, got: " + _currentToken.tokenLiteral);
            return nullptr;
        }

        const auto body = ParseBlockStatement();
        if (body == nullptr) {
            return nullptr;
        }

        funcExpression->body = body;
        return funcExpression;
    }

    bool Parser::CurrentTokenIs(const Token::TokenType tokenType) const {
        return _currentToken.tokenType == tokenType;
    }

    bool Parser::PeekTokenIs(const Token::TokenType tokenType) const {
        return _peekToken.tokenType == tokenType;
    }

    std::shared_ptr<IntLiteral> Parser::ParseIntegerLiteral() const {
        auto integerLiteral = std::make_shared<IntLiteral>();
        integerLiteral->token = _currentToken;
        integerLiteral->value = std::strtoll(_currentToken.tokenLiteral.c_str(), nullptr, 10);

        return integerLiteral;
    }

    std::shared_ptr<FloatLiteral> Parser::ParseFloatLiteral() const {
        auto floatLiteral = std::make_shared<FloatLiteral>();
        floatLiteral->token = _currentToken;
        floatLiteral->value = std::strtod(_currentToken.tokenLiteral.c_str(), nullptr);

        return floatLiteral;
    }

    std::shared_ptr<BoolLiteral> Parser::ParseBoolLiteral() const {
        auto boolLiteral = std::make_shared<BoolLiteral>();
        boolLiteral->token = _currentToken;
        boolLiteral->value = _currentToken.tokenType == Token::True;

        return boolLiteral;
    }

    std::shared_ptr<CharLiteral> Parser::ParseCharLiteral() {
        auto charLiteral = std::make_shared<CharLiteral>();
        charLiteral->token = _currentToken;
        if (!ExpectPeek(Token::Ident)) {
            return nullptr;
        }

        if (_currentToken.tokenLiteral.length() != 1) {
            return nullptr;
        }

        charLiteral->value = _currentToken.tokenLiteral[0];
        if (!ExpectPeek(Token::Apostrophe)) {
            return nullptr;
        }

        return charLiteral;
    }

    std::shared_ptr<StringLiteral> Parser::ParseStringLiteral() {
        auto stringLiteral = std::make_shared<StringLiteral>();
        stringLiteral->token = _currentToken;
        if (!ExpectPeek(Token::Ident)) {
            return nullptr;
        }

        stringLiteral->value = _currentToken.tokenLiteral;
        if (!ExpectPeek(Token::Quote)) {
            return nullptr;
        }

        return stringLiteral;
    }
} // ast