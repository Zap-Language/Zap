#include <fstream>
#include <iostream>
#include <memory>
#include "lib/lexer/lexer.h"
#include "lib/parser/Parser.h"
#include "lib/parser/SemanticAnalyzer.h"
#include "lib/bytecode/compiler.h"
#include "lib/bytecode/disassembler.h"

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file> [-o <output_file>] [-d]" << std::endl;
        std::cout << "-d - readable bytecode" << std::endl;
        return 1;
    }

    std::string inputPath;
    std::string outputPath = "output.bc";
    bool debug = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-o") {
            if (i + 1 < argc) {
                outputPath = argv[++i];
            } else {
                std::cerr << "Error: -o requires an argument" << std::endl;
                return 1;
            }
        } else if (arg == "-d") {
            debug = true;
        } else if (inputPath.empty()) {
            inputPath = arg;
        }
    }

    if (inputPath.empty()) {
        std::cerr << "Error: No input file specified" << std::endl;
        return 1;
    }

    std::ifstream ifs(inputPath);
    if (!ifs.is_open()) {
        std::cerr << "Error: Cannot open file " << inputPath << std::endl;
        return 1;
    }

    std::string input((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    if (input.empty()) {
        std::cerr << "Error: File is empty" << std::endl;
        return 1;
    }

    Lexer lexer(input);
    ast::Parser parser(lexer);
    auto program = parser.ParseProgram();

    if (!parser._errors.empty()) {
        parser.PrintErrors();
        return 1;
    }

    ast::SemanticAnalyzer analyzer;
    std::shared_ptr<ast::Program> sharedProgram(program.release());
    if (!analyzer.AnalyzeProgram(sharedProgram)) {
        analyzer.PrintErrors();
        return 1;
    }

    bytecode::Compiler compiler;
    auto chunk = compiler.Compile(*sharedProgram);

    if (!chunk) {
        std::cerr << "Failed to compile program" << std::endl;
        return 1;
    }

    std::ofstream ofs(outputPath, std::ios::binary);
    if (!ofs.is_open()) {
        std::cerr << "Error: Cannot open output file " << outputPath << std::endl;
        return 1;
    }

    chunk->Serialize(ofs);

    std::cout << "Successfully compiled '" << inputPath << "' to '" << outputPath << "'" << std::endl;

    if (debug) {
        std::ofstream debugOutput("out.dbg", std::ios::binary);
        bytecode::Disassembler::Disassemble(*chunk, debugOutput);
    }

    return 0;
}