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
            auto curChar = this->_curChar;
            if (*this->_peakChar == '=') {
                this->ReadChar();
                token = Token::Token(Token::Assign, std::string(curChar, this->_curChar + 1));
                break;
            }

            token = Token::Token(Token::Illegal, std::string(1, *this->_curChar));
            break;
        }
        case '.': {
            token = Token::Token(Token::Dot, std::string(1, *this->_curChar));
            break;
        }
        case ';': {
            token = Token::Token(Token::Semicolon, std::string(1, *this->_curChar));
            break;
        }
        case ',': {
            token = Token::Token(Token::Comma, std::string(1, *this->_curChar));
            break;
        }
        case '!': {
            token = Token::Token(Token::Bang, std::string(1, *this->_curChar));
            break;
        }
        case '=': {
        if (*this->_peakChar == '=') {
            this->ReadChar();
            token = Token::Token(Token::Equal, "==");
            break;
        }
        token = Token::Token(Token::Assign, "=");
        break;
    }
        case '+':
            token = Token::Token(Token::Plus, "+");
            break;
        case '-':
            token = Token::Token(Token::Minus, "-");
            break;
        case '*': {
            if (*this->_peakChar == '*') {
                this->ReadChar();
                token = Token::Token(Token::Power, "**");
                break;
            }
            token = Token::Token(Token::Asterisk, "*");
            break;
        }
        case '/':
            token = Token::Token(Token::Slash, "/");
            break;
        case '%':
            token = Token::Token(Token::Modulo, "%");
            break;
        case '|': {
            if (*this->_peakChar == '|') {
                this->ReadChar();
                token = Token::Token(Token::Or, "||");
                break;
            }

            token = Token::Token(Token::Illegal, std::string(1, *this->_curChar));
            break;
        }
        case '&': {
            if (*this->_peakChar == '&') {
                this->ReadChar();
                token = Token::Token(Token::And, "&&");
                break;
            }

            token = Token::Token(Token::Illegal, std::string(1, *this->_curChar));
            break;
        }
        case '#':
            token = Token::Token(Token::NotEqual, "#");
            break;
        case '(':
            token = Token::Token(Token::LParen, "(");
            break;
        case ')':
            token = Token::Token(Token::RParen, ")");
            break;
        case '{':
            token = Token::Token(Token::LBrace, "{");
            break;
        case '}':
            token = Token::Token(Token::RBrace, "}");
            break;
        case '[':
            token = Token::Token(Token::LBracket, "[");
            break;
        case ']':
            token = Token::Token(Token::RBracket, "]");
            break;
        case '\n':
            token = Token::Token(Token::NewLine, "\n");
            break;
        case 0:
            token = Token::Token(Token::Eof, std::string(1, *this->_curChar));
            break;
        default:
            if(isalpha(*this->_curChar)) {
                const std::string word = this->ReadIdent();
                if (_identMap.contains(word)) {
                    token = Token::Token(_identMap.at(word), word);
                } else {
                    token = Token::Token(Token::Ident, word);
                }

                return token;
            }
            if (isdigit(*_curChar)) {
                const std::string number = this->ReadNumber();
                if (number.find('.') != std::string::npos) {
                    token = Token::Token(Token::Float, number);
                } else {
                    token = Token::Token(Token::Integer, number);
                }

                return token;
            }

            token = Token::Token(Token::Illegal, std::string(1, *this->_curChar));
            break;
    }

    this->ReadChar();
    return token;
}

void Lexer::SkipWhitespaces() {
    do {
        switch (*this->_curChar) {
            case ' ':
            case '\r':
            case '\t':
                break;
            default:
                return;
        }
    } while(this->ReadChar());
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
    if (*this->_curChar == '.') {
        this->ReadChar();
        while (isdigit(*this->_curChar)) this->ReadChar();
    }

    return {curIter, this->_curChar};
}

Lexer::Lexer(std::string &input)  {
    _input = std::string(input.begin(), input.end());
    _curChar = _input.begin();
    _peakChar = _curChar+1;
    _identMap = {
        {"func", Token::Func},
        {"let", Token::Let},
        {"return", Token::Return},
        {"if", Token::If},
        {"else", Token::Else},
        {"for", Token::For},
        {"while", Token::While},

        {"true", Token::True},
        {"false", Token::False},

        {"int", Token::IntType},
        {"float", Token::FloatType},
        {"char", Token::CharType},
        {"string", Token::StringType},
        {"bool", Token::BoolType},

        {"len", Token::Len},
        {"read", Token::Read},
        {"print", Token::Print}
    };
}

bool Lexer::ReadChar() {
    if (*_curChar == 0) {
        return false;
    }

    _curChar = _peakChar;
    ++_peakChar;
    return true;
}
