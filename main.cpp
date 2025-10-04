#include <fstream>
#include <iostream>
#include <lexer/lexer.h>

int main(int argc, char** argv) {
//    std::ifstream ifs("test.z");
    std::string input;
    std::getline(std::cin, input);
    Lexer lexer(input);

    Token::Token curToken;
    do {
        curToken = lexer.NextToken();
        std::cout << curToken.tokenType << ' ';
    } while(curToken.tokenType != Token::Eof);
}