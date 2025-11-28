#include <fstream>
#include <iostream>
#include <lexer/lexer.h>

#include "parser/Parser.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
        return 1;
    }

    std::ifstream ifs(argv[1]);
    std::istreambuf_iterator<char> begin(ifs);
    std::istreambuf_iterator<char> end;

    std::string input(begin, end);
    Lexer lexer(input);
    ast::Parser parser(lexer);
    auto program = parser.ParseProgram();

    std::cout << program->String() << std::endl;
}
