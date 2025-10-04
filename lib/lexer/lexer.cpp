#include "lexer.h"

Token::Token Lexer::NextToken() {
    this->SkipWhitespaces();

    Token::Token token;
    switch (*this->_curChar) {
        case '>':
            token = Token::Token(Token::Greater, std::string(1, *this->_curChar));
            break;
        case '<':
            token = Token::Token(Token::Less, std::string(1, *this->_curChar));
            break;
        case ':': {
            char curChar = *this->_curChar;
            if (*this->_peakChar == '=') {
                this->ReadChar();
                token = Token::Token(Token::Assign, std::string(2, curChar + *this->_curChar));
                break;
            }

            token = Token::Token(Token::Illegal, std::string(1, *this->_curChar));
            break;
        }
        case 0:
            token = Token::Token(Token::Eof, std::string(1, *this->_curChar));
            break;
        default:
            if(isalpha(*this->_curChar)) {
                token = Token::Token(Token::Ident, this->ReadIdent());
                return token;
            } else if (isdigit(*this->_curChar)) {
                token = Token::Token(Token::Integer, this->ReadNumber());
                return token;
            }

            token = Token::Token(Token::Illegal, std::string(1, *this->_curChar));
            break;
    }

    this->ReadChar();
    return token;
}

void Lexer::SkipWhitespaces() {
    while(*this->_curChar != 0) {
        switch (*this->_curChar) {
            case ' ':
            case '\r':
            case '\t':
                this->ReadChar();
                break;
            default:
                return;
        }
    }
}

std::string Lexer::ReadIdent() {
    auto curIter = this->_curChar;
    while(isalpha(*this->_curChar) || isdigit(*this->_curChar)) {
        this->ReadChar();
    }

    return {curIter, this->_curChar};
}

std::string Lexer::ReadNumber() {
    auto curIter = this->_curChar;
    while (isdigit(*this->_curChar)) this->ReadChar();

    return {curIter, this->_curChar};
}
