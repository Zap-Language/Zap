#include <fstream>
#include <iostream>
#include <lexer/lexer.h>

#include "parser/Parser.h"
#include "parser/SemanticAnalyzer.h"

int main(int argc, char **argv) {
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

    parser.PrintErrors();

    if (!program) {
        std::cerr << "Failed to parse program" << std::endl;
        return 1;
    }

    ast::SemanticAnalyzer analyzer;
    std::shared_ptr<ast::Program> sharedProgram(program.release());
    bool success = analyzer.AnalyzeProgram(sharedProgram);

    std::cout << "Semantic analysis successful: " << (success ? "YES" : "NO") << std::endl;
    analyzer.PrintErrors();

    if (success) {
        std::cout << "\nSymbol table" << std::endl;
        analyzer.PrintSymbolTable();
    }

    return success ? 0 : 1;
}
