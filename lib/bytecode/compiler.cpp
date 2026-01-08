#include "compiler.h"
#include <unordered_set>
#include "parser/ast/BlockStatement.h"
#include "parser/ast/BoolLiteral.h"
#include "parser/ast/CallExpression.h"
#include "parser/ast/CharLiteral.h"
#include "parser/ast/DataTypeArray.h"
#include "parser/ast/FloatLiteral.h"
#include "parser/ast/Identifier.h"
#include "parser/ast/InfixExpression.h"
#include "parser/ast/IntLiteral.h"
#include "parser/ast/LetStatement.h"
#include "parser/ast/PrefixExpression.h"
#include "parser/ast/StringLiteral.h"

#include "bytecode_chunk.h"
#include "parser/ast/AssignStatement.h"
#include "parser/ast/DataTypeVoid.h"
#include "parser/ast/ElseStatement.h"
#include "parser/ast/ExpressionStatement.h"
#include "parser/ast/ForStatement.h"
#include "parser/ast/FuncStatement.h"
#include "parser/ast/IfStatement.h"
#include "parser/ast/ReturnStatement.h"
#include "parser/ast/WhileStatement.h"

namespace bytecode {

Compiler::Compiler()
    : _chunk(nullptr)
    , _globalCount(0)
    , _currentFunction(nullptr)
    , _scopeDepth(0) {
}

std::unique_ptr<BytecodeChunk> Compiler::Compile(const ast::Program& program) {
    _chunk = std::make_unique<BytecodeChunk>();
    _globals.clear();
    _globalCount = 0;
    _scopeDepth = 0;

    for (const auto& stmt : program.statements) {
        CompileStatement(*stmt);
    }

    _chunk->EmitOpCode(OpCode::HALT);

    return std::move(_chunk);
}

void Compiler::CompileStatement(const ast::StatementNode& stmt) {
    if (auto* let = dynamic_cast<const ast::LetStatement*>(&stmt)) {
        CompileLetStatement(*let);
    }
    else if (auto* assign = dynamic_cast<const ast::AssignStatement*>(&stmt)) {
        CompileAssignStatement(*assign);
    }
    else if (auto* exprStmt = dynamic_cast<const ast::ExpressionStatement*>(&stmt)) {
        CompileExpression(*exprStmt->expression);
        _chunk->EmitOpCode(OpCode::POP);
    }
    else if (auto* block = dynamic_cast<const ast::BlockStatement*>(&stmt)) {
        CompileBlockStatement(*block);
    }
    else if (auto* ifStmt = dynamic_cast<const ast::IfStatement*>(&stmt)) {
        CompileIfStatement(*ifStmt);
    }
    else if (auto* elseStmt = dynamic_cast<const ast::ElseStatement*>(&stmt)) {
        CompileElseStatement(*elseStmt);
    }
    else if (auto* whileStmt = dynamic_cast<const ast::WhileStatement*>(&stmt)) {
        CompileWhileStatement(*whileStmt);
    }
    else if (auto* forStmt = dynamic_cast<const ast::ForStatement*>(&stmt)) {
        CompileForStatement(*forStmt);
    }
    else if (auto* funcStmt = dynamic_cast<const ast::FuncStatement*>(&stmt)) {
        CompileFuncStatement(*funcStmt);
    }
    else if (auto* retStmt = dynamic_cast<const ast::ReturnStatement*>(&stmt)) {
        CompileReturnStatement(*retStmt);
    }
    else {
        throw CompilerError("Unknown statement type");
    }
}

void Compiler::CompileLetStatement(const ast::LetStatement& stmt) {
    CompileExpression(*stmt.value);

    const std::string& varName = trim(stmt.name->String());
    ValueType varType = ConvertType(*stmt.name->type);

    if (IsInFunction()) {
        uint32_t index = DeclareLocal(varName, varType);
        _chunk->EmitOpCode(OpCode::STORE_LOCAL);
        _chunk->EmitUint32(index);
    } else {
        uint32_t index = DeclareGlobal(varName);
        _chunk->EmitOpCode(OpCode::STORE_GLOBAL);
        _chunk->EmitUint32(index);
    }
}

void Compiler::CompileAssignStatement(const ast::AssignStatement& stmt) {
    if (auto* ident = dynamic_cast<ast::Identifier*>(stmt.target.get())) {
        CompileExpression(*stmt.expression);
        std::string varName = ident->TokenLiteral();
        EmitStore(varName);
        return;
    }

    if (auto* indexExpr = dynamic_cast<ast::IndexExpression*>(stmt.target.get())) {
        CompileExpression(*indexExpr->left);
        CompileExpression(*indexExpr->index);
        CompileExpression(*stmt.expression);
        _chunk->EmitOpCode(OpCode::ARRAY_SET);
        return;
    }

    throw CompilerError("Unsupported assignment target");
}

void Compiler::CompileBlockStatement(const ast::BlockStatement& stmt) {
    BeginScope();

    for (const auto& s : stmt.statements) {
        CompileStatement(*s);
    }

    EndScope();
}

    void Compiler::CompileIfStatement(const ast::IfStatement& stmt) {
    /*
     * if condition {
     *     then-block
     * } else {
     *     else-block (может быть вложенным IfStatement для else-if)
     * }
     *
     * Генерируем:
     *     <condition>
     *     JMP_IF_FALSE else_label
     *     <then-block>
     *     JMP end
     * else_label:
     *     [else-block]
     * end:
     */

    CompileExpression(*stmt.condition);

    size_t jumpIfFalse = EmitJump(OpCode::JMP_IF_FALSE);

    CompileStatement(*stmt.thenStatement);

    if (stmt.elseStatement) {
        size_t jumpEnd = EmitJump(OpCode::JMP);

        PatchJump(jumpIfFalse);

        CompileStatement(*stmt.elseStatement);

        PatchJump(jumpEnd);
    } else {
        PatchJump(jumpIfFalse);
    }
}

void Compiler::CompileElseStatement(const ast::ElseStatement& stmt) {
    if (stmt.stmt) {
        CompileStatement(*stmt.stmt);
    }
}
void Compiler::CompileWhileStatement(const ast::WhileStatement& stmt) {
    /*
     * while condition {
     *     body
     * }
     *
     * Генерируем:
     * loop_start:
     *     <condition>
     *     JMP_IF_FALSE loop_end
     *     <body>
     *     JMP loop_start
     * loop_end:
     */

    size_t loopStart = _chunk->CurrentOffset();

    _loopStack.push({loopStart, {}});

    CompileExpression(*stmt.condition);

    size_t exitJump = EmitJump(OpCode::JMP_IF_FALSE);

    CompileStatement(*stmt.stmt);

    EmitLoop(loopStart);

    PatchJump(exitJump);

    LoopContext& loop = _loopStack.top();
    for (size_t breakPatch : loop.breakPatches) {
        PatchJump(breakPatch);
    }

    _loopStack.pop();
}

void Compiler::CompileForStatement(const ast::ForStatement& stmt) {
    /*
     * for letStatement; condition; postStatement {
     *     body
     * }
     *
     * Эквивалентно:
     * {
     *     letStatement
     *     while condition {
     *         body
     *         postStatement
     *     }
     * }
     */

    BeginScope();

    if (stmt.letStatement) {
        CompileLetStatement(*stmt.letStatement);
    }

    size_t loopStart = _chunk->CurrentOffset();

    _loopStack.push({0, {}});

    size_t exitJump = 0;
    if (stmt.condition) {
        CompileExpression(*stmt.condition);
        exitJump = EmitJump(OpCode::JMP_IF_FALSE);
    }

    CompileStatement(*stmt.stmt);

    _loopStack.top().continueTarget = _chunk->CurrentOffset();

    if (stmt.postStatement) {
        CompileStatement(*stmt.postStatement);
    }

    EmitLoop(loopStart);

    if (stmt.condition) {
        PatchJump(exitJump);
    }

    for (size_t breakPatch : _loopStack.top().breakPatches) {
        PatchJump(breakPatch);
    }

    _loopStack.pop();

    EndScope();
}

void Compiler::CompileFuncStatement(const ast::FuncStatement& stmt) {
    const std::string& funcName = trim(stmt.name->String());
    ValueType returnType = ConvertType(*stmt.returnType);

    FunctionInfo func;
    func.name = funcName;
    func.returnType = returnType;
    func.paramCount = static_cast<uint8_t>(stmt.arguments->arguments.size());

    uint32_t funcIndex = _chunk->AddFunction(func);

    _globals[funcName] = funcIndex;

    size_t skipJump = EmitJump(OpCode::JMP);

    _chunk->GetFunction(funcIndex).codeOffset = static_cast<uint32_t>(_chunk->CurrentOffset());

    _currentFunction = std::make_unique<FunctionContext>();
    _currentFunction->name = funcName;
    _currentFunction->funcIndex = funcIndex;
    _currentFunction->scopeDepth = 0;
    _currentFunction->returnType = returnType;

    BeginScope();

    for (const auto& arg : stmt.arguments->arguments) {
        ValueType argType = ConvertType(*arg->type);
        DeclareLocal(trim(arg->String()), argType);
        _chunk->GetFunction(funcIndex).paramTypes.push_back(argType);
    }

    for (const auto& s : stmt.body->statements) {
        CompileStatement(*s);
    }

    bool hasExplicitReturn = false;
    if (!stmt.body->statements.empty()) {
        auto* lastStmt = stmt.body->statements.back().get();
        hasExplicitReturn = dynamic_cast<const ast::ReturnStatement*>(lastStmt) != nullptr;
    }

    if (!hasExplicitReturn) {
        if (returnType == ValueType::VOID) {
            _chunk->EmitOpCode(OpCode::RETURN_VOID);
        } else {
            _chunk->EmitOpCode(OpCode::PUSH_NULL);
            _chunk->EmitOpCode(OpCode::RETURN);
        }
    }

    _chunk->GetFunction(funcIndex).localCount =
        static_cast<uint8_t>(_currentFunction->locals.size());
    _chunk->GetFunction(funcIndex).codeLength =
        static_cast<uint32_t>(_chunk->CurrentOffset() - _chunk->GetFunction(funcIndex).codeOffset);

    EndScope();
    _currentFunction.reset();

    PatchJump(skipJump);
}

void Compiler::CompileReturnStatement(const ast::ReturnStatement& stmt) {
    if (stmt.expression != nullptr) {
        std::string debruh = trim(stmt.expression->String());
        CompileExpression(*stmt.expression);
        _chunk->EmitOpCode(OpCode::RETURN);
    } else {
        _chunk->EmitOpCode(OpCode::RETURN_VOID);
    }
}


void Compiler::CompileExpression(const ast::ExpressionNode& expr) {
    if (auto* intLit = dynamic_cast<const ast::IntLiteral*>(&expr)) {
        CompileIntegerLiteral(*intLit);
    }
    else if (auto* floatLit = dynamic_cast<const ast::FloatLiteral*>(&expr)) {
        CompileFloatLiteral(*floatLit);
    }
    else if (auto* boolLit = dynamic_cast<const ast::BoolLiteral*>(&expr)) {
        CompileBoolLiteral(*boolLit);
    }
    else if (auto* charLit = dynamic_cast<const ast::CharLiteral*>(&expr)) {
        CompileCharLiteral(*charLit);
    }
    else if (auto* strLit = dynamic_cast<const ast::StringLiteral*>(&expr)) {
        CompileStringLiteral(*strLit);
    }
    else if (auto* ident = dynamic_cast<const ast::Identifier*>(&expr)) {
        CompileIdentifier(*ident);
    }
    else if (auto* prefix = dynamic_cast<const ast::PrefixExpression*>(&expr)) {
        CompilePrefixExpression(*prefix);
    }
    else if (auto* infix = dynamic_cast<const ast::InfixExpression*>(&expr)) {
        CompileInfixExpression(*infix);
    }
    else if (auto* call = dynamic_cast<const ast::CallExpression*>(&expr)) {
        CompileCallExpression(*call);
    }
    else if (auto* arr = dynamic_cast<const ast::ArrayLiteral*>(&expr)) {
        CompileArrayLiteral(*arr);
    }
    else if (auto* idx = dynamic_cast<const ast::IndexExpression*>(&expr)) {
        CompileIndexExpression(*idx);
    }
    else if (auto* funcExpr = dynamic_cast<const ast::FuncExpression*>(&expr)) {
        CompileFuncExpression(*funcExpr);
    }
    else {
        throw CompilerError("Unknown expression type");
    }
}

void Compiler::CompileIntegerLiteral(const ast::IntLiteral& lit) const {
    _chunk->EmitOpCode(OpCode::PUSH_INT);
    _chunk->EmitInt64(lit.value);
}

void Compiler::CompileFloatLiteral(const ast::FloatLiteral& lit) const {
    _chunk->EmitOpCode(OpCode::PUSH_FLOAT);
    _chunk->EmitDouble(static_cast<double>(lit.value));
}

void Compiler::CompileBoolLiteral(const ast::BoolLiteral& lit) const {
    _chunk->EmitOpCode(OpCode::PUSH_BOOL);
    _chunk->EmitByte(lit.value ? 1 : 0);
}

void Compiler::CompileCharLiteral(const ast::CharLiteral& lit) const {
    _chunk->EmitOpCode(OpCode::PUSH_CHAR);
    _chunk->EmitByte(static_cast<uint8_t>(lit.value));
}

void Compiler::CompileStringLiteral(const ast::StringLiteral& lit) const {
    uint32_t index = _chunk->AddString(lit.value);
    _chunk->EmitOpCode(OpCode::PUSH_STRING);
    _chunk->EmitUint32(index);
}

void Compiler::CompileIdentifier(const ast::Identifier& ident) const {
    EmitLoad(trim(ident.token.tokenLiteral));
}

void Compiler::CompilePrefixExpression(const ast::PrefixExpression& expr) {
    CompileExpression(*expr.rightExpression);

    const std::string& op = expr.token.tokenLiteral;

    if (op == "-") {
        _chunk->EmitOpCode(OpCode::NEG);
    }
    else if (op == "!") {
        _chunk->EmitOpCode(OpCode::NOT);
    }
    else {
        throw CompilerError("Unknown prefix operator: " + op);
    }
}

void Compiler::CompileInfixExpression(const ast::InfixExpression& expr) {
    const std::string& op = expr.token.tokenLiteral;
    if (op == "&&") {
        CompileExpression(*expr.leftExpression);
        _chunk->EmitOpCode(OpCode::DUP);
        size_t shortCircuit = EmitJump(OpCode::JMP_IF_FALSE);
        _chunk->EmitOpCode(OpCode::POP);
        CompileExpression(*expr.rightExpression);
        PatchJump(shortCircuit);
        return;
    }

    if (op == "||") {
        CompileExpression(*expr.leftExpression);
        _chunk->EmitOpCode(OpCode::DUP);
        size_t shortCircuit = EmitJump(OpCode::JMP_IF_TRUE);
        _chunk->EmitOpCode(OpCode::POP);
        CompileExpression(*expr.rightExpression);
        PatchJump(shortCircuit);
        return;
    }

    CompileExpression(*expr.leftExpression);
    CompileExpression(*expr.rightExpression);

    if (op == "+") {
        _chunk->EmitOpCode(OpCode::ADD);
    }
    else if (op == "-") {
        _chunk->EmitOpCode(OpCode::SUB);
    }
    else if (op == "*") {
        _chunk->EmitOpCode(OpCode::MUL);
    }
    else if (op == "/") {
        _chunk->EmitOpCode(OpCode::DIV);
    }
    else if (op == "%") {
        _chunk->EmitOpCode(OpCode::MOD);
    }
    else if (op == "**") {
        _chunk->EmitOpCode(OpCode::POW);
    }
    else if (op == "==") {
        _chunk->EmitOpCode(OpCode::CMP_EQ);
    }
    else if (op == "#" || op == "!=") {
        _chunk->EmitOpCode(OpCode::CMP_NE);
    }
    else if (op == "<") {
        _chunk->EmitOpCode(OpCode::CMP_LT);
    }
    else if (op == ">") {
        _chunk->EmitOpCode(OpCode::CMP_GT);
    }
    else if (op == "<=") {
        _chunk->EmitOpCode(OpCode::CMP_LE);
    }
    else if (op == ">=") {
        _chunk->EmitOpCode(OpCode::CMP_GE);
    }
    else {
        throw CompilerError("Unknown infix operator: " + op);
    }
}

void Compiler::CompileCallExpression(const ast::CallExpression& expr) {
    std::string funcName;
    if (auto* ident = dynamic_cast<ast::Identifier*>(expr.function.get())) {
        funcName = trim(ident->String());
    } else {
        throw CompilerError("Invalid function call target");
    }

    for (const auto& arg : expr.arguments) {
        CompileExpression(*arg);
    }

    const auto argCount = static_cast<uint8_t>(expr.arguments.size());

    if (IsBuiltinFunction(funcName)) {
        BuiltinFunction builtin = GetBuiltinFunction(funcName);
        _chunk->EmitOpCode(OpCode::CALL_BUILTIN);
        _chunk->EmitByte(static_cast<uint8_t>(builtin));
        _chunk->EmitByte(argCount);
        return;
    }

    auto it = _globals.find(funcName);
    if (it == _globals.end()) {
        throw CompilerError("Unknown function: " + funcName);
    }

    _chunk->EmitOpCode(OpCode::CALL);
    _chunk->EmitUint32(it->second);
    _chunk->EmitByte(argCount);
}

void Compiler::CompileArrayLiteral(const ast::ArrayLiteral& arr) {
    ValueType elemType = ValueType::INT;
    if (arr.elementType) {
        elemType = ConvertType(*arr.elementType);
    }

    CompileExpression(*arr.count);
    _chunk->EmitOpCode(OpCode::NEW_ARRAY);
    _chunk->EmitByte(static_cast<uint8_t>(elemType));
}

void Compiler::CompileIndexExpression(const ast::IndexExpression& expr) {
    CompileExpression(*expr.left);
    CompileExpression(*expr.index);
    _chunk->EmitOpCode(OpCode::ARRAY_GET);
}

void Compiler::CompileFuncExpression(const ast::FuncExpression& expr) {
    static uint32_t lambdaCounter = 0;
    std::string lambdaName = "__lambda_" + std::to_string(lambdaCounter++);

    ValueType returnType = ConvertType(*expr.returnType);

    FunctionInfo func;
    func.name = lambdaName;
    func.returnType = returnType;
    func.paramCount = static_cast<uint8_t>(expr.arguments->arguments.size());

    uint32_t funcIndex = _chunk->AddFunction(func);

    size_t skipJump = EmitJump(OpCode::JMP);

    _chunk->GetFunction(funcIndex).codeOffset = static_cast<uint32_t>(_chunk->CurrentOffset());

    auto savedFunction = std::move(_currentFunction);

    _currentFunction = std::make_unique<FunctionContext>();
    _currentFunction->name = lambdaName;
    _currentFunction->funcIndex = funcIndex;
    _currentFunction->scopeDepth = 0;
    _currentFunction->returnType = returnType;

    BeginScope();

    for (const auto& arg : expr.arguments->arguments) {
        ValueType argType = ConvertType(*arg->type);
        DeclareLocal(arg->TokenLiteral(), argType);
        _chunk->GetFunction(funcIndex).paramTypes.push_back(argType);
    }

    for (const auto& s : expr.body->statements) {
        CompileStatement(*s);
    }

    if (returnType == ValueType::VOID) {
        _chunk->EmitOpCode(OpCode::RETURN_VOID);
    } else {
        _chunk->EmitOpCode(OpCode::PUSH_NULL);
        _chunk->EmitOpCode(OpCode::RETURN);
    }

    _chunk->GetFunction(funcIndex).localCount =
        static_cast<uint8_t>(_currentFunction->locals.size());
    _chunk->GetFunction(funcIndex).codeLength =
        static_cast<uint32_t>(_chunk->CurrentOffset() - _chunk->GetFunction(funcIndex).codeOffset);

    EndScope();
    _currentFunction = std::move(savedFunction);

    PatchJump(skipJump);

    _chunk->EmitOpCode(OpCode::PUSH_FUNC);
    _chunk->EmitUint32(funcIndex);
}

uint32_t Compiler::DeclareLocal(const std::string& name, ValueType type) const {
    if (!_currentFunction) {
        throw CompilerError("Cannot declare local variable outside of function");
    }

    for (const auto& local : _currentFunction->locals) {
        if (local.name == name && local.scopeDepth == _currentFunction->scopeDepth) {
            throw CompilerError("Variable already declared in this scope: " + name);
        }
    }

    const auto index = static_cast<uint32_t>(_currentFunction->locals.size());
    _currentFunction->locals.push_back({name, index, _currentFunction->scopeDepth, type});

    return index;
}

int32_t Compiler::ResolveLocal(const std::string& name) const {
    if (!_currentFunction) {
        return -1;
    }

    for (int32_t i = static_cast<int32_t>(_currentFunction->locals.size()) - 1; i >= 0; --i) {
        if (_currentFunction->locals[i].name == name) {
            return i;
        }
    }

    return -1;
}

uint32_t Compiler::DeclareGlobal(const std::string& name) {
    auto it = _globals.find(name);
    if (it != _globals.end()) {
        throw CompilerError("Global variable already declared: " + name);
    }

    uint32_t index = _globalCount++;
    _globals[name] = index;
    return index;
}

int32_t Compiler::ResolveGlobal(const std::string& name) const {
    auto it = _globals.find(name);
    if (it != _globals.end()) {
        return static_cast<int32_t>(it->second);
    }
    return -1;
}

void Compiler::EmitLoad(const std::string& name) const {
    int32_t localIndex = ResolveLocal(name);
    if (localIndex >= 0) {
        _chunk->EmitOpCode(OpCode::LOAD_LOCAL);
        _chunk->EmitUint32(static_cast<uint32_t>(localIndex));
        return;
    }

    int32_t globalIndex = ResolveGlobal(name);
    if (globalIndex >= 0) {
        _chunk->EmitOpCode(OpCode::LOAD_GLOBAL);
        _chunk->EmitUint32(static_cast<uint32_t>(globalIndex));
        return;
    }

    throw CompilerError("Undefined variable: " + name);
}

void Compiler::EmitStore(const std::string& name) const {
    int32_t localIndex = ResolveLocal(name);
    if (localIndex >= 0) {
        _chunk->EmitOpCode(OpCode::STORE_LOCAL);
        _chunk->EmitUint32(static_cast<uint32_t>(localIndex));
        return;
    }

    int32_t globalIndex = ResolveGlobal(name);
    if (globalIndex >= 0) {
        _chunk->EmitOpCode(OpCode::STORE_GLOBAL);
        _chunk->EmitUint32(static_cast<uint32_t>(globalIndex));
        return;
    }

    throw CompilerError("Undefined variable: " + name);
}

size_t Compiler::EmitJump(OpCode jumpOp) const {
    _chunk->EmitOpCode(jumpOp);
    size_t patchPosition = _chunk->CurrentOffset();
    _chunk->EmitUint32(0);
    return patchPosition;
}

void Compiler::PatchJump(size_t jumpPosition) const {
    const auto target = static_cast<uint32_t>(_chunk->CurrentOffset());
    _chunk->PatchUint32(jumpPosition, target);
}

void Compiler::EmitLoop(size_t loopStart) const {
    _chunk->EmitOpCode(OpCode::JMP);
    _chunk->EmitUint32(static_cast<uint32_t>(loopStart));
}

void Compiler::BeginScope() {
    _scopeDepth++;
    if (_currentFunction) {
        _currentFunction->scopeDepth++;
    }
}

void Compiler::EndScope() {
    _scopeDepth--;

    if (_currentFunction) {
        _currentFunction->scopeDepth--;

        while (!_currentFunction->locals.empty() &&
               _currentFunction->locals.back().scopeDepth > _currentFunction->scopeDepth) {
            _currentFunction->locals.pop_back();
            _chunk->EmitOpCode(OpCode::POP);
        }
    }
}

ValueType Compiler::ConvertType(ast::DataType& type) {
    return ConvertType(type.Type());
}

ValueType Compiler::ConvertType(ast::TypeDataType type) {
    switch (type) {
        case ast::TypeDataType::Int:    return ValueType::INT;
        case ast::TypeDataType::Float:  return ValueType::FLOAT;
        case ast::TypeDataType::Bool:   return ValueType::BOOL;
        case ast::TypeDataType::Char:   return ValueType::CHAR;
        case ast::TypeDataType::String: return ValueType::STRING;
        case ast::TypeDataType::Array:  return ValueType::ARRAY;
        case ast::TypeDataType::Void:   return ValueType::VOID;
        case ast::TypeDataType::Func:   return ValueType::FUNCTION;
        default:
            throw CompilerError("Unknown type");
    }
}

bool Compiler::IsBuiltinFunction(const std::string& name) {
    static const std::unordered_set<std::string> builtins = {
        "print", "len", "read", "int", "float", "bool", "char", "string"
    };
    return builtins.contains(name);
}

BuiltinFunction Compiler::GetBuiltinFunction(const std::string& name) {
    static const std::unordered_map<std::string, BuiltinFunction> builtins = {
        {"print",  BuiltinFunction::PRINT},
        {"len",    BuiltinFunction::LEN},
        {"read",   BuiltinFunction::READ},
        {"int",    BuiltinFunction::CAST_INT},
        {"float",  BuiltinFunction::CAST_FLOAT},
        {"bool",   BuiltinFunction::CAST_BOOL},
        {"char",   BuiltinFunction::CAST_CHAR},
        {"string", BuiltinFunction::CAST_STRING},
    };

    auto it = builtins.find(name);
    if (it != builtins.end()) {
        return it->second;
    }

    throw CompilerError("Unknown builtin function: " + name);
}

}
