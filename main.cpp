#include <fstream>
#include <iostream>
#include <memory>
#include "lib/lexer/lexer.h"
#include "lib/parser/Parser.h"
#include "lib/parser/SemanticAnalyzer.h"
#include "lib/bytecode/compiler.h"
#include "lib/bytecode/disassembler.h"
#include "jit/vm.h"
#include "jit/jit.h"

using namespace jit;

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file>" << std::endl;
        return 1;
    }

    std::ifstream ifs(argv[1]);
    if (!ifs.is_open()) {
        std::cerr << "Error: Cannot open file " << argv[1] << std::endl;
        return 1;
    }
    
    std::istreambuf_iterator<char> begin(ifs);
    std::istreambuf_iterator<char> end;
    std::string input(begin, end);
    
    if (input.empty()) {
        std::cerr << "Error: File is empty" << std::endl;
        return 1;
    }

    Lexer lexer(input);
    ast::Parser parser(lexer);
    auto program = parser.ParseProgram();

    if (!parser._errors.empty()) {
        std::cerr << "Parse errors:" << std::endl;
        parser.PrintErrors();
        return 1;
    }

    if (!program) {
        std::cerr << "Failed to parse program" << std::endl;
        return 1;
    }

    ast::SemanticAnalyzer analyzer;
    std::shared_ptr<ast::Program> sharedProgram(program.release());
    bool success = analyzer.AnalyzeProgram(sharedProgram);

    if (!analyzer.GetErrors().empty()) {
        std::cerr << "Semantic errors:" << std::endl;
        analyzer.PrintErrors();
        return 1;
    }

    if (!success) {
        std::cerr << "Semantic analysis failed" << std::endl;
        return 1;
    }

    bytecode::Compiler compiler;
    auto chunk = compiler.Compile(*sharedProgram);
    
    if (!chunk) {
        std::cerr << "Failed to compile program" << std::endl;
        return 1;
    }

    VM vm;
    JITCompiler jit(&vm);
    vm.setJITCompiler(&jit);
    vm.loadChunk(std::move(chunk));
    vm.run();
    
    if (vm.getStatus() != VM::Status::OK) {
        std::cerr << "Runtime error occurred" << std::endl;
        return 1;
    }
    
    std::cout << std::endl;
    std::cout << "Program executed successfully!" << std::endl;
    
    return 0;
}
