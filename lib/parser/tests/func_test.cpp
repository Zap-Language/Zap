#include <gtest/gtest.h>
#include <parser/parser.h>
#include <lexer/lexer.h>
#include <utils.h>
#include <ast/DataTypeInt.h>
#include <ast/DataTypeVoid.h>

std::unique_ptr<ast::Program> parse(std::string& input) {
    const Lexer lexer(input);
    ast::Parser parser(lexer);
    return parser.ParseProgram();
}

TEST(FuncTest, TestEmptyFunc) {
    std::string input = "func f(){}";
    auto program = parse(input);
    ASSERT_TRUE(program != nullptr);

    ASSERT_EQ(program->statements.size(), 1);
    auto stmt = program->statements.at(0);
    ASSERT_TRUE(stmt != nullptr);

    auto fnStmt = dynamic_cast<ast::FuncStatement*>(stmt.get());
    ASSERT_TRUE(fnStmt != nullptr);
    ASSERT_EQ(fnStmt->returnType, ast::VOID);
    ASSERT_EQ(fnStmt->arguments->arguments.size(), 0);
    ASSERT_EQ(fnStmt->body->statements.size(), 0);
    ASSERT_EQ(trim(fnStmt->name->String()), "f");
}

TEST(FuncTest, TestArgsParsing) {
    std::string input = "func f(int arg1, int arg2){}";
    auto program = parse(input);
    ASSERT_TRUE(program != nullptr);

    ASSERT_EQ(program->statements.size(), 1);
    auto stmt = program->statements.at(0);
    ASSERT_TRUE(stmt != nullptr);

    auto fnStmt = dynamic_cast<ast::FuncStatement*>(stmt.get());
    ASSERT_TRUE(fnStmt != nullptr);
    ASSERT_EQ(fnStmt->returnType, ast::VOID);
    ASSERT_EQ(fnStmt->arguments->arguments.size(), 2);
    ASSERT_EQ(fnStmt->arguments->arguments.at(0)->type->Type(), ast::Int);
    ASSERT_EQ(trim(fnStmt->arguments->arguments.at(0)->String()), "arg1");
    ASSERT_EQ(fnStmt->arguments->arguments.at(1)->type->Type(), ast::Int);
    ASSERT_EQ(trim(fnStmt->arguments->arguments.at(1)->String()), "arg2");

    ASSERT_EQ(fnStmt->body->statements.size(), 0);
}

TEST(FuncTest, TestReturnTypeParsing) {
    std::string input = "func f() int {}";
    auto program = parse(input);
    ASSERT_TRUE(program != nullptr);

    ASSERT_EQ(program->statements.size(), 1);
    auto stmt = program->statements.at(0);
    ASSERT_TRUE(stmt != nullptr);

    auto fnStmt = dynamic_cast<ast::FuncStatement*>(stmt.get());
    ASSERT_TRUE(fnStmt != nullptr);
    ASSERT_TRUE(fnStmt->returnType != nullptr);

    ASSERT_EQ(fnStmt->returnType->Type(), ast::Int);
    ASSERT_EQ(fnStmt->arguments->arguments.size(), 0);

    ASSERT_EQ(fnStmt->body->statements.size(), 0);
}
