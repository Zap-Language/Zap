#include "bytecode/compiler.h"
#include "bytecode/disassembler.h"
#include "parser/ast/Ast.h"
#include "parser/parser.h"
#include "lexer/lexer.h"
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <cassert>

#include "parser/SemanticAnalyzer.h"

using namespace bytecode;

TEST(TestCompiler, SimpleFunction) {
    auto program = std::make_unique<ast::Program>();

    std::string source = R"(
        func main(int b, int g) int {
            return b + g
        }
    )";
    std::cout << source << '\n';

    Lexer lex(source);
    ast::Parser pars(lex);

    const std::shared_ptr<ast::Program> parsedProgram = pars.ParseProgram();

    if (!pars._errors.empty()) {
        for (const auto& err : pars._errors) {
            std::cerr << "Parse error: " << err << "\n";
        }
        ASSERT_TRUE(false && "Parse errors");
    }
    ast::SemanticAnalyzer anal;

    anal.AnalyzeProgram(parsedProgram);
    if (anal.GetErrors().size() != 0)
    {
        for (auto i : anal.GetErrors())
        {
            std::cerr << i.message << "\n";
        }

        ASSERT_TRUE(false && "Errors");
    }
    Compiler compiler;
    auto chunk = compiler.Compile(*parsedProgram);

    std::cout << "\n";
    Disassembler::Disassemble(*chunk, std::cout);

    std::cout << "PASSED\n";
    ASSERT_TRUE(true);
}


TEST(TestCompiler, SimpleCalculations) {
    auto program = std::make_unique<ast::Program>();

    std::string source = R"(
            let x = 2 + 2 * 2
    )";
    std::cout << source << '\n';
    Lexer lex(source);
    ast::Parser pars(lex);

    const std::shared_ptr<ast::Program> parsedProgram = pars.ParseProgram();

    if (!pars._errors.empty()) {
        for (const auto& err : pars._errors) {
            std::cerr << "Parse error: " << err << "\n";
        }
        ASSERT_TRUE(false && "Parse errors");
    }
    ast::SemanticAnalyzer anal;

    anal.AnalyzeProgram(parsedProgram);
    if (anal.GetErrors().size() != 0)
    {
        for (auto i : anal.GetErrors())
        {
            std::cerr << i.message << "\n";
        }

        ASSERT_TRUE(false && "Errors");
    }
    Compiler compiler;
    auto chunk = compiler.Compile(*parsedProgram);

    std::cout << "\n";
    Disassembler::Disassemble(*chunk, std::cout);

    std::cout << "PASSED\n";
    ASSERT_TRUE(true);
}


TEST(TestCompiler, MultiLinesCode) {
    auto program = std::make_unique<ast::Program>();

    std::string source = R"(
            func main() int {
                let x = 2 + 2 * 2
                let b = x - 2
                return x - b
            }

            let result = main()
    )";
    std::cout << source << '\n';
    Lexer lex(source);
    ast::Parser pars(lex);

    const std::shared_ptr<ast::Program> parsedProgram = pars.ParseProgram();

    if (!pars._errors.empty()) {
        for (const auto& err : pars._errors) {
            std::cerr << "Parse error: " << err << "\n";
        }
        ASSERT_TRUE(false && "Parse errors");
    }
    ast::SemanticAnalyzer anal;

    anal.AnalyzeProgram(parsedProgram);
    if (anal.GetErrors().size() != 0)
    {
        for (auto i : anal.GetErrors())
        {
            std::cerr << i.message << "\n";
        }

        ASSERT_TRUE(false && "Errors");
    }
    Compiler compiler;
    auto chunk = compiler.Compile(*parsedProgram);

    std::cout << "\n";
    Disassembler::Disassemble(*chunk, std::cout);

    std::cout << "PASSED\n";
    ASSERT_TRUE(true);
}


TEST(TestCompiler, IfStatementCheck) {
    auto program = std::make_unique<ast::Program>();

    std::string source = R"(
    let x = 1
    if (x < 10) {
         x = x + 5
    } else {
         x = x - 2
    }
    )";
    std::cout << source << '\n';
    Lexer lex(source);
    ast::Parser pars(lex);

    const std::shared_ptr<ast::Program> parsedProgram = pars.ParseProgram();

    if (!pars._errors.empty()) {
        for (const auto& err : pars._errors) {
            std::cerr << "Parse error: " << err << "\n";
        }
        ASSERT_TRUE(false && "Parse errors");
    }
    ast::SemanticAnalyzer anal;

    anal.AnalyzeProgram(parsedProgram);
    if (anal.GetErrors().size() != 0)
    {
        for (auto i : anal.GetErrors())
        {
            std::cerr << i.message << "\n";
        }

        ASSERT_TRUE(false && "Errors");
    }
    Compiler compiler;
    auto chunk = compiler.Compile(*parsedProgram);

    std::cout << "\n";
    Disassembler::Disassemble(*chunk, std::cout);

    std::cout << "PASSED\n";
    ASSERT_TRUE(true);
}

TEST(TestCompiler, ForTest) {
    auto program = std::make_unique<ast::Program>();

    std::string source = R"(

    for (let x = 0; x < 5; x = x + 1) {
        let n = x + 2
    }
    )";
    std::cout << source << '\n';
    Lexer lex(source);
    ast::Parser pars(lex);

    const std::shared_ptr<ast::Program> parsedProgram = pars.ParseProgram();

    if (!pars._errors.empty()) {
        for (const auto& err : pars._errors) {
            std::cerr << "Parse error: " << err << "\n";
        }
        ASSERT_TRUE(false && "Parse errors");
    }
    ast::SemanticAnalyzer anal;

    anal.AnalyzeProgram(parsedProgram);
    if (anal.GetErrors().size() != 0)
    {
        for (auto i : anal.GetErrors())
        {
            std::cerr << i.message << "\n";
        }

        ASSERT_TRUE(false && "Errors");
    }
    Compiler compiler;
    auto chunk = compiler.Compile(*parsedProgram);

    std::cout << "\n";
    Disassembler::Disassemble(*chunk, std::cout);

    std::cout << "PASSED\n";
    ASSERT_TRUE(true);
}


TEST(TestCompiler, WhileTest) {
    auto program = std::make_unique<ast::Program>();

    std::string source = R"(
    let x = 0
    while(x < 5) {
        let n = x + 2
        x = x + 1
    }
    )";
    std::cout << source << '\n';
    Lexer lex(source);
    ast::Parser pars(lex);

    const std::shared_ptr<ast::Program> parsedProgram = pars.ParseProgram();

    if (!pars._errors.empty()) {
        for (const auto& err : pars._errors) {
            std::cerr << "Parse error: " << err << "\n";
        }
        ASSERT_TRUE(false && "Parse errors");
    }
    ast::SemanticAnalyzer anal;

    anal.AnalyzeProgram(parsedProgram);
    if (anal.GetErrors().size() != 0)
    {
        for (auto i : anal.GetErrors())
        {
            std::cerr << i.message << "\n";
        }

        ASSERT_TRUE(false && "Errors");
    }
    Compiler compiler;
    auto chunk = compiler.Compile(*parsedProgram);

    std::cout << "\n";
    Disassembler::Disassemble(*chunk, std::cout);

    std::cout << "PASSED\n";
    ASSERT_TRUE(true);
}

