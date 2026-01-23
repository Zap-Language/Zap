#pragma once

#include "bytecode_chunk.h"
#include "opcodes.h"
#include "value_type.h"

#include "parser/ast/Ast.h"
#include "parser/ast/symbols/SymbolTable.h"

#include <memory>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "parser/ast/AssignStatement.h"
#include "parser/ast/BoolLiteral.h"
#include "parser/ast/CallExpression.h"
#include "parser/ast/CharLiteral.h"
#include "parser/ast/DataType.h"
#include "parser/ast/ArrayLiteral.h"
#include "parser/ast/IndexExpression.h"
#include "parser/ast/FieldAccessExpression.h"
#include "parser/ast/NewExpression.h"
#include "parser/ast/StructStatement.h"
#include "parser/ast/ElseStatement.h"
#include "parser/ast/FloatLiteral.h"
#include "parser/ast/ForStatement.h"
#include "parser/ast/FuncExpression.h"
#include "parser/ast/FuncStatement.h"
#include "parser/ast/IfStatement.h"
#include "parser/ast/InfixExpression.h"
#include "parser/ast/IntLiteral.h"
#include "parser/ast/LetStatement.h"
#include "parser/ast/PrefixExpression.h"
#include "parser/ast/Program.h"
#include "parser/ast/ReturnStatement.h"
#include "parser/ast/StringLiteral.h"
#include "parser/ast/WhileStatement.h"

namespace bytecode {

class CompilerError : public std::runtime_error {
public:
    explicit CompilerError(const std::string& message)
        : std::runtime_error("Compile error: " + message) {}
};

struct LocalVariable {
    std::string name;
    uint32_t index;
    uint32_t scopeDepth;
    ValueType type;
};

struct LoopContext {
    size_t continueTarget;
    std::vector<size_t> breakPatches;
};

struct FunctionContext {
    std::string name;
    uint32_t funcIndex;
    std::vector<LocalVariable> locals;
    uint32_t scopeDepth;
    ValueType returnType;
    std::string methodStructName;
};

class Compiler {
public:
    Compiler();
    std::unique_ptr<BytecodeChunk> Compile(const ast::Program& program);
private:
    std::unique_ptr<BytecodeChunk> _chunk;
    std::unordered_map<std::string, uint32_t> _globals;
    std::unordered_map<std::string, uint32_t> _functions;
    uint32_t _globalCount;
    std::unique_ptr<FunctionContext> _currentFunction;
    std::stack<LoopContext> _loopStack;
    uint32_t _scopeDepth;
    void CompileStatement(const ast::StatementNode& stmt);
    void CompileLetStatement(const ast::LetStatement& stmt);
    void CompileAssignStatement(const ast::AssignStatement& stmt);
    void CompileBlockStatement(const ast::BlockStatement& stmt);
    void CompileIfStatement(const ast::IfStatement& stmt);
    void CompileElseStatement(const ast::ElseStatement& stmt);
    void CompileWhileStatement(const ast::WhileStatement& stmt);
    void CompileForStatement(const ast::ForStatement& stmt);

    void CompileFuncStatement(const ast::FuncStatement& stmt);
    void CompileReturnStatement(const ast::ReturnStatement& stmt);
    void CompileFieldAccessExpression(const ast::FieldAccessExpression& expr);
    void CompileNewExpression(const ast::NewExpression& expr);
    void CompileStructStatement(const ast::StructStatement& stmt);
   
    void CompileExpression(const ast::ExpressionNode& expr);
    void CompileIntegerLiteral(const ast::IntLiteral& lit) const;
    void CompileFloatLiteral(const ast::FloatLiteral& lit) const;
    void CompileBoolLiteral(const ast::BoolLiteral& lit) const;
    void CompileCharLiteral(const ast::CharLiteral& lit) const;
    void CompileStringLiteral(const ast::StringLiteral& lit) const;
    void CompileIdentifier(const ast::Identifier& ident) const;
    void CompilePrefixExpression(const ast::PrefixExpression& expr);
    void CompileInfixExpression(const ast::InfixExpression& expr);
    void CompileCallExpression(const ast::CallExpression& expr);
    void CompileArrayLiteral(const ast::ArrayLiteral& arr);
    void CompileIndexExpression(const ast::IndexExpression& expr);
   
    void CompileFuncExpression(const ast::FuncExpression& expr);
   
    uint32_t DeclareLocal(const std::string& name, ValueType type) const;
    int32_t ResolveLocal(const std::string& name) const;
    int32_t ResolveFunction(const std::string& name) const;
    uint32_t DeclareGlobal(const std::string& name);
    int32_t ResolveGlobal(const std::string& name) const;

    void EmitLoad(const std::string& name) const;
    void EmitStore(const std::string& name) const;

   
    size_t EmitJump(OpCode jumpOp) const;
    void PatchJump(size_t jumpPosition) const;
    std::unordered_map<std::string, uint32_t> _structIndices;
    std::unordered_map<std::string, std::unordered_map<std::string, uint32_t>> _structFieldIndices;
    std::unordered_map<std::string, std::unordered_map<std::string, uint32_t>> _structMethodIndices;
    void EmitLoop(size_t loopStart) const;

   
    void BeginScope();
    void EndScope();


    static ValueType ConvertType(ast::DataType& type);

    static ValueType ConvertType(ast::TypeDataType type);


    static bool IsBuiltinFunction(const std::string& name);

    static BuiltinFunction GetBuiltinFunction(const std::string& name);

   
    bool IsInFunction() const { return _currentFunction != nullptr; }
    bool IsGlobalScope() const { return _scopeDepth == 0 && !IsInFunction(); }
};

}
