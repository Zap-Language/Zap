#include "SemanticAnalyzer.h"
#include "ast/DataTypeInt.h"
#include "ast/DataTypeFloat.h"
#include "ast/DataTypeString.h"
#include "ast/DataTypeBool.h"
#include "ast/DataTypeChar.h"
#include "ast/DataTypeVoid.h"
#include "ast/BlockStatement.h"
#include <iostream>
#include <sstream>

namespace ast {
    SemanticAnalyzer::SemanticAnalyzer() = default;

    bool SemanticAnalyzer::AnalyzeProgram(const std::shared_ptr<Program>& program) {
        if (!program) {
            AddError("Program is null");
            return false;
        }

        errors.clear();
        symbolTable.ClearErrors();

        bool success = true;
        for (const auto& stmt: program->statements) {
            success &= AnalyzeStatement(stmt);
        }

        for (const auto& symbolError: symbolTable.GetErrors()) {
            AddError(symbolError.message, symbolError.symbolName);
        }

        return success && errors.empty();
    }

    const std::vector<SemanticError>& SemanticAnalyzer::GetErrors() const {
        return errors;
    }

    const SymbolTable& SemanticAnalyzer::GetSymbolTable() const {
        return symbolTable;
    }

    void SemanticAnalyzer::PrintErrors() const {
        if (!errors.empty()) {
            std::cout << std::string(8, '=') << "SEMANTIC ERRORS:" << std::string(8, '=') << std::endl;
        }

        for (const auto& error: errors) {
            std::cout << "Error: " << error.message;
            if (!error.location.empty()) {
                std::cout << " (at: " << error.location << ")";
            }
            std::cout << std::endl;
        }
    }

    void SemanticAnalyzer::PrintSymbolTable() const {
        std::cout << symbolTable.ToString() << std::endl;
    }

    bool SemanticAnalyzer::AnalyzeStatement(const std::shared_ptr<StatementNode>& stmt) {
        if (!stmt) {
            return true;
        }

        if (auto funcStmt = std::dynamic_pointer_cast<FuncStatement>(stmt)) {
            return AnalyzeFuncStatement(funcStmt);
        }
        if (auto letStmt = std::dynamic_pointer_cast<LetStatement>(stmt)) {
            return AnalyzeLetStatement(letStmt);
        }
        if (auto retStmt = std::dynamic_pointer_cast<ReturnStatement>(stmt)) {
            return AnalyzeReturnStatement(retStmt);
        }

        AddError("Unknown statement type");
        return false;
    }

    bool SemanticAnalyzer::AnalyzeFuncStatement(const std::shared_ptr<FuncStatement>& func) {
        if (!func) {
            AddError("Function statement is null");
            return false;
        }
        std::string previousScopeName = symbolTable.GetCurrentScopeName();
        std::string fullFuncName = previousScopeName.empty() ? 
            func->name->TokenLiteral() : 
            previousScopeName + "::" + func->name->TokenLiteral();

        if (!symbolTable.DeclareFunction(func->name->TokenLiteral(), func)) {
            AddError("Failed to declare function '" + func->name->TokenLiteral() + "'");
        }
        symbolTable.EnterScope();
        symbolTable.SetCurrentScopeName(fullFuncName);
        auto previousFunctionReturnType = currentFunctionReturnType;
        currentFunctionReturnType = func->returnType;
        bool success = true;
        if (func->arguments) {
            for (const auto& param: func->arguments->arguments) {
                if (!symbolTable.DeclareParameter(param->TokenLiteral(), param->type)) {
                    AddError("Failed to declare parameter '" + param->TokenLiteral() + "'", func->name->TokenLiteral());
                    success = false;
                }
            }
        }
        if (func->body) {
            for (const auto& stmt: func->body->statements) {
                success &= AnalyzeStatement(stmt);
            }
        }
        symbolTable.ExitScope();
        symbolTable.SetCurrentScopeName(previousScopeName);
        currentFunctionReturnType = previousFunctionReturnType;
        return success;
    }

    bool SemanticAnalyzer::AnalyzeLetStatement(const std::shared_ptr<LetStatement>& let) {
        if (!let) {
            AddError("Let statement is null");
            return false;
        }

        if (!let->value || !let->name) {
            AddError("Let statement missing name or value");
            return false;
        }
        auto valueType = AnalyzeExpression(let->value);
        if (!valueType) {
            AddError("Could not determine type of expression", let->name->TokenLiteral());
            return false;
        }
        if (let->name->type) {
            if (!TypesCompatible(let->name->type, valueType)) {
                AddError("Type mismatch: variable '" + let->name->TokenLiteral() +
                         "' declared as " + let->name->type->String() +
                         " but assigned " + valueType->String());
                return false;
            }
        } else {
            let->name->type = valueType;
        }
        if (!symbolTable.DeclareVariable(let->name->TokenLiteral(), let->name->type)) {
            AddError("Failed to declare variable '" + let->name->TokenLiteral() + "'");
            return false;
        }

        return true;
    }

    bool SemanticAnalyzer::AnalyzeReturnStatement(const std::shared_ptr<ReturnStatement>& ret) {
        if (!ret) {
            AddError("Return statement is null");
            return false;
        }

        if (!currentFunctionReturnType) {
            AddError("Return statement outside of function");
            return false;
        }

        std::shared_ptr<DataType> returnType;
        if (ret->expression) {
            returnType = AnalyzeExpression(ret->expression);
            if (!returnType) {
                AddError("Could not determine type of return expression");
                return false;
            }
        } else {
            returnType = VOID;
        }

        if (!TypesCompatible(currentFunctionReturnType, returnType)) {
            AddError("Return type mismatch: expected " + currentFunctionReturnType->String() +
                     " but got " + returnType->String());
            return false;
        }

        return true;
    }

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeExpression(const std::shared_ptr<ExpressionNode>& expr) {
        if (!expr) {
            return nullptr;
        }

        if (auto identifier = std::dynamic_pointer_cast<Identifier>(expr)) {
            return AnalyzeIdentifier(identifier);
        }
        if (auto intLit = std::dynamic_pointer_cast<IntLiteral>(expr)) {
            return AnalyzeIntLiteral(intLit);
        }
        if (auto floatLit = std::dynamic_pointer_cast<FloatLiteral>(expr)) {
            return AnalyzeFloatLiteral(floatLit);
        }
        if (auto boolLit = std::dynamic_pointer_cast<BoolLiteral>(expr)) {
            return AnalyzeBoolLiteral(boolLit);
        }
        if (auto charLit = std::dynamic_pointer_cast<CharLiteral>(expr)) {
            return AnalyzeCharLiteral(charLit);
        }
        if (auto stringLit = std::dynamic_pointer_cast<StringLiteral>(expr)) {
            return AnalyzeStringLiteral(stringLit);
        }
        if (auto callExpr = std::dynamic_pointer_cast<CallExpression>(expr)) {
            return AnalyzeCallExpression(callExpr);
        }
        if (auto infixExpr = std::dynamic_pointer_cast<InfixExpression>(expr)) {
            return AnalyzeInfixExpression(infixExpr);
        }
        if (auto prefixExpr = std::dynamic_pointer_cast<PrefixExpression>(expr)) {
            return AnalyzePrefixExpression(prefixExpr);
        }

        AddError("Unknown expression type");
        return nullptr;
    }

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeCallExpression(const std::shared_ptr<CallExpression>& call) {
        if (!call || !call->function) {
            return nullptr;
        }

        auto functionId = std::dynamic_pointer_cast<Identifier>(call->function);
        if (!functionId) {
            AddError("Function call must use identifier");
            return nullptr;
        }

        std::string funcName = functionId->TokenLiteral();

        if (symbolTable.IsBuiltinFunction(funcName)) {
            return AnalyzeBuiltinCall(funcName, call->arguments);
        }

        auto funcDecl = symbolTable.LookupFunction(funcName);
        if (!funcDecl) {
            AddError("Unknown function '" + funcName + "'");
            return nullptr;
        }

        size_t expectedArgs = funcDecl->arguments ? funcDecl->arguments->arguments.size() : 0;
        size_t actualArgs = call->arguments.size();

        if (expectedArgs != actualArgs) {
            AddError("Function '" + funcName + "' expects " + std::to_string(expectedArgs) +
                     " arguments but got " + std::to_string(actualArgs));
            return nullptr;
        }

        if (funcDecl->arguments) {
            for (size_t i = 0; i < expectedArgs; ++i) {
                auto expectedType = funcDecl->arguments->arguments[i]->type;
                auto actualType = AnalyzeExpression(call->arguments[i]);

                if (!actualType) {
                    AddError("Could not determine type of argument " + std::to_string(i + 1));
                    return nullptr;
                }

                if (!TypesCompatible(expectedType, actualType)) {
                    AddError("Argument " + std::to_string(i + 1) + " type mismatch: expected " +
                             expectedType->String() + " but got " + actualType->String());
                    return nullptr;
                }
            }
        }

        return funcDecl->returnType;
    }

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeInfixExpression(const std::shared_ptr<InfixExpression>& infix) {
        if (!infix) {
            return nullptr;
        }

        auto leftType = AnalyzeExpression(infix->leftExpression);
        auto rightType = AnalyzeExpression(infix->rightExpression);

        if (!leftType || !rightType) {
            AddError("Could not determine operand types for infix expression");
            return nullptr;
        }

        return GetResultTypeForInfixOperation(infix->token.tokenLiteral, leftType, rightType);
    }

    std::shared_ptr<DataType>
    SemanticAnalyzer::AnalyzePrefixExpression(const std::shared_ptr<PrefixExpression>& prefix) {
        if (!prefix) {
            return nullptr;
        }

        auto operandType = AnalyzeExpression(prefix->rightExpression);
        if (!operandType) {
            AddError("Could not determine operand type for prefix expression");
            return nullptr;
        }

        return GetResultTypeForPrefixOperation(prefix->token.tokenLiteral, operandType);
    }

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeIdentifier(const std::shared_ptr<Identifier>& identifier) {
        if (!identifier) {
            return nullptr;
        }

        std::string name = identifier->TokenLiteral();
        auto type = symbolTable.LookupVariable(name);

        if (!type) {
            AddError("Undefined variable '" + name + "'");
            return nullptr;
        }

        if (!identifier->type) {
            identifier->type = type;
        }

        return type;
    }

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeIntLiteral(const std::shared_ptr<IntLiteral>&) {
        return INT;
    }

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeFloatLiteral(const std::shared_ptr<FloatLiteral>&) {
        return FLOAT;
    }

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeBoolLiteral(const std::shared_ptr<BoolLiteral>&) {
        return BOOL;
    }

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeCharLiteral(const std::shared_ptr<CharLiteral>&) {
        return CHAR;
    }

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeStringLiteral(const std::shared_ptr<StringLiteral>&) {
        return STRING;
    }

    bool SemanticAnalyzer::TypesEqual(const std::shared_ptr<DataType>& a, const std::shared_ptr<DataType>& b) const {
        if (!a || !b) return false;
        return a->Type() == b->Type();
    }

    bool SemanticAnalyzer::TypesCompatible(const std::shared_ptr<DataType>& expected,
                                           const std::shared_ptr<DataType>& actual) const {
        return TypesEqual(expected, actual);
    }

    std::shared_ptr<DataType> SemanticAnalyzer::GetResultTypeForInfixOperation(
            const std::string& operator_,
            const std::shared_ptr<DataType>& left,
            const std::shared_ptr<DataType>& right) const {

        if (operator_ == "+" || operator_ == "-" || operator_ == "*" || operator_ == "/" || operator_ == "%") {
            if (left->Type() == TypeDataType::String || right->Type() == TypeDataType::String) {
                if (operator_ == "+") {
                    return STRING;
                } else {
                    AddError("Invalid operation '" + operator_ + "' on string");
                    return nullptr;
                }
            }

            if (left->Type() == TypeDataType::Float || right->Type() == TypeDataType::Float) {
                return FLOAT;
            }
            if (left->Type() == TypeDataType::Int && right->Type() == TypeDataType::Int) {
                return INT;
            }

            AddError("Invalid arithmetic operation '" + operator_ + "' on types " +
                     left->String() + " and " + right->String());
            return nullptr;
        }

        if (operator_ == "==" || operator_ == "#" || operator_ == "<" || operator_ == ">") {
            if (TypesCompatible(left, right)) {
                return BOOL;
            }
            AddError("Cannot compare types " + left->String() + " and " + right->String());
            return nullptr;
        }

        if (operator_ == "&&" || operator_ == "||") {
            if (left->Type() == TypeDataType::Bool && right->Type() == TypeDataType::Bool) {
                return BOOL;
            }
            AddError("Logical operations require boolean operands");
            return nullptr;
        }

        AddError("Unknown infix operator '" + operator_ + "'");
        return nullptr;
    }

    std::shared_ptr<DataType> SemanticAnalyzer::GetResultTypeForPrefixOperation(
            const std::string& operator_,
            const std::shared_ptr<DataType>& operand) const {

        if (operator_ == "!") {
            if (operand->Type() == TypeDataType::Bool) {
                return BOOL;
            }
            AddError("Logical negation requires boolean operand");
            return nullptr;
        }

        if (operator_ == "-") {
            if (operand->Type() == TypeDataType::Int) {
                return INT;
            }
            if (operand->Type() == TypeDataType::Float) {
                return FLOAT;
            }
            AddError("Negation requires numeric operand");
            return nullptr;
        }

        AddError("Unknown prefix operator '" + operator_ + "'");
        return nullptr;
    }

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeBuiltinCall(const std::string& functionName,
                                                                   const std::vector<std::shared_ptr<ExpressionNode>>& args) {
        if (functionName == "print") {
            if (args.size() != 1) {
                AddError("print() expects exactly 1 argument");
                return nullptr;
            }
            AnalyzeExpression(args[0]);
            return VOID;
        }
        if (functionName == "len") {
            if (args.size() != 1) {
                AddError("len() expects exactly 1 argument");
                return nullptr;
            }
            auto argType = AnalyzeExpression(args[0]);
            if (!argType || argType->Type() != TypeDataType::Array) {
                AddError("len() requires array argument");
                return nullptr;
            }
            return INT;
        }

        if (functionName == "read") {
            if (!args.empty()) {
                AddError("read() expects no arguments");
                return nullptr;
            }
            return STRING;
        }

        if (functionName == "int" || functionName == "float" || functionName == "string" ||
            functionName == "bool" || functionName == "char") {
            if (args.size() != 1) {
                AddError(functionName + "() expects exactly 1 argument");
                return nullptr;
            }
            AnalyzeExpression(args[0]);

            if (functionName == "int") return INT;
            if (functionName == "float") return FLOAT;
            if (functionName == "string") return STRING;
            if (functionName == "bool") return BOOL;
            if (functionName == "char") return CHAR;
        }

        AddError("Unknown built-in function '" + functionName + "'");
        return nullptr;
    }

    void SemanticAnalyzer::AddError(const std::string& message, const std::string& location) const {
        errors.emplace_back(message, location);
    }
}