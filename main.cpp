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
    std::string input;
    while (std::getline(ifs, input)) {
        Lexer lexer(input);
        ast::Parser parser(lexer);
        auto program = parser.ParseProgram();

        std::cout << program->String() << std::endl;
    }
}
