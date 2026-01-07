#include "SemanticAnalyzer.h"
#include "ast/DataTypeInt.h"
#include "ast/DataTypeFloat.h"
#include "ast/DataTypeString.h"
#include "ast/DataTypeBool.h"
#include "ast/DataTypeChar.h"
#include "ast/DataTypeVoid.h"
#include "ast/BlockStatement.h"
#include <iostream>

#include "ast/ElseStatement.h"

namespace ast {
    SemanticAnalyzer::SemanticAnalyzer() = default;

    bool SemanticAnalyzer::AnalyzeProgram(const std::shared_ptr<Program> &program) {
        if (!program) {
            AddError("Program is null");
            return false;
        }

        errors.clear();
        symbolTable.ClearErrors();

        bool success = true;
        for (const auto &stmt: program->statements) {
            success &= AnalyzeStatement(stmt);
        }

        for (const auto &symbolError: symbolTable.GetErrors()) {
            AddError(symbolError.message, symbolError.symbolName);
        }

        return success && errors.empty();
    }

    const std::vector<SemanticError> &SemanticAnalyzer::GetErrors() const {
        return errors;
    }

    const SymbolTable &SemanticAnalyzer::GetSymbolTable() const {
        return symbolTable;
    }

    void SemanticAnalyzer::PrintErrors() const {
        if (!errors.empty()) {
            std::cout << std::string(8, '=') << "SEMANTIC ERRORS:" << std::string(8, '=') << std::endl;
        }

        for (const auto &error: errors) {
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

    bool SemanticAnalyzer::AnalyzeStatement(const std::shared_ptr<StatementNode> &stmt) {
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
        if (auto assignStmt = std::dynamic_pointer_cast<AssignStatement>(stmt)) {
            return AnalyzeAssignStatement(assignStmt);
        }
        if (auto ifStmt = std::dynamic_pointer_cast<IfStatement>(stmt)) {
            return AnalyzeIfStatement(ifStmt);
        }
        if (auto whileStmt = std::dynamic_pointer_cast<WhileStatement>(stmt)) {
            return AnalyzeWhileStatement(whileStmt);
        }
        if (auto forStmt = std::dynamic_pointer_cast<ForStatement>(stmt)) {
            return AnalyzeForStatement(forStmt);
        }
        if (auto exprStmt = std::dynamic_pointer_cast<ExpressionStatement>(stmt)) {
            return AnalyzeExpressionStatement(exprStmt);
        }
        if (auto blockStmt = std::dynamic_pointer_cast<BlockStatement>(stmt)) {
            bool success = true;
            symbolTable.EnterScope();
            for (const auto &s: blockStmt->statements) {
                success &= AnalyzeStatement(s);
            }
            symbolTable.ExitScope();
            return success;
        }
        if (auto elseStmt = std::dynamic_pointer_cast<ElseStatement>(stmt)) {
            return AnalyzeStatement(elseStmt->stmt);
        }

        AddError("Unknown statement type");
        return false;
    }

    bool SemanticAnalyzer::AnalyzeFuncStatement(const std::shared_ptr<FuncStatement> &func) {
        if (!func) {
            AddError("Function statement is null");
            return false;
        }
        std::string previousScopeName = symbolTable.GetCurrentScopeName();
        std::string fullFuncName = previousScopeName.empty()
                                       ? func->name->TokenLiteral()
                                       : previousScopeName + "::" + func->name->TokenLiteral();

        if (!symbolTable.DeclareFunction(func->name->TokenLiteral(), func)) {
            AddError("Failed to declare function '" + func->name->TokenLiteral() + "'");
        }
        symbolTable.EnterScope();
        symbolTable.SetCurrentScopeName(fullFuncName);
        auto previousFunctionReturnType = currentFunctionReturnType;
        currentFunctionReturnType = func->returnType;
        bool success = true;
        if (func->arguments) {
            for (const auto &param: func->arguments->arguments) {
                if (!symbolTable.DeclareParameter(param->TokenLiteral(), param->type)) {
                    AddError("Failed to declare parameter '" + param->TokenLiteral() + "'", func->name->TokenLiteral());
                    success = false;
                }
            }
        }
        if (func->body) {
            for (const auto &stmt: func->body->statements) {
                success &= AnalyzeStatement(stmt);
            }
        }
        symbolTable.ExitScope();
        symbolTable.SetCurrentScopeName(previousScopeName);
        currentFunctionReturnType = previousFunctionReturnType;
        return success;
    }

    bool SemanticAnalyzer::AnalyzeLetStatement(const std::shared_ptr<LetStatement> &let) {
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

    bool SemanticAnalyzer::AnalyzeReturnStatement(const std::shared_ptr<ReturnStatement> &ret) {
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

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeExpression(const std::shared_ptr<ExpressionNode> &expr) {
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
        if (auto arrayLit = std::dynamic_pointer_cast<ArrayLiteral>(expr)) {
            return AnalyzeArrayLiteral(arrayLit);
        }
        if (auto indexExpr = std::dynamic_pointer_cast<IndexExpression>(expr)) {
            return AnalyzeIndexExpression(indexExpr);
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

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeCallExpression(const std::shared_ptr<CallExpression> &call) {
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

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeInfixExpression(const std::shared_ptr<InfixExpression> &infix) {
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
    SemanticAnalyzer::AnalyzePrefixExpression(const std::shared_ptr<PrefixExpression> &prefix) {
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

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeIdentifier(const std::shared_ptr<Identifier> &identifier) const {
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

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeIntLiteral(const std::shared_ptr<IntLiteral> &) {
        return INT;
    }

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeFloatLiteral(const std::shared_ptr<FloatLiteral> &) {
        return FLOAT;
    }

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeBoolLiteral(const std::shared_ptr<BoolLiteral> &) {
        return BOOL;
    }

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeCharLiteral(const std::shared_ptr<CharLiteral> &) {
        return GetCharType();
    }

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeStringLiteral(const std::shared_ptr<StringLiteral> &) {
        return GetStringType();
    }

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeArrayLiteral(const std::shared_ptr<ArrayLiteral> &arrayLit) {
        if (!arrayLit) {
            return nullptr;
        }

        auto count = AnalyzeExpression(arrayLit->count);
        if (!count) {
            return nullptr;
        }

        if (count->Type() != Int) {
            AddError("Array literal size type expected to be int");
            return nullptr;
        }

        if (arrayLit->elements.empty()) {
            if (arrayLit->elementType) {
                return std::make_shared<DataTypeArray>(arrayLit->elementType);
            }
            AddError("Array literal cannot be empty without element type");
            return nullptr;
        }

        auto firstType = AnalyzeExpression(arrayLit->elements.front());
        if (!firstType) {
            return nullptr;
        }

        if (arrayLit->elementType && !TypesCompatible(arrayLit->elementType, firstType)) {
            AddError("Array literal element does not match declared type " + arrayLit->elementType->String());
            return nullptr;
        }

        for (size_t i = 1; i < arrayLit->elements.size(); ++i) {
            auto elemType = AnalyzeExpression(arrayLit->elements[i]);
            if (!elemType) {
                return nullptr;
            }
            if (!TypesCompatible(firstType, elemType)) {
                AddError("Array literal elements have incompatible types: " +
                         firstType->String() + " and " + elemType->String());
                return nullptr;
            }
        }

        arrayLit->elementType = arrayLit->elementType ? arrayLit->elementType : firstType;
        return std::make_shared<DataTypeArray>(arrayLit->elementType);
    }

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeIndexExpression(
        const std::shared_ptr<IndexExpression> &indexExpr) {
        if (!indexExpr) {
            return nullptr;
        }

        auto arrayType = AnalyzeExpression(indexExpr->left);
        auto idxType = AnalyzeExpression(indexExpr->index);

        if (!arrayType || !idxType) {
            return nullptr;
        }

        if (arrayType->Type() != TypeDataType::Array) {
            AddError("Indexing requires array type");
            return nullptr;
        }

        if (idxType->Type() != TypeDataType::Int) {
            AddError("Array index must be int");
            return nullptr;
        }

        auto *arrayDataType = dynamic_cast<DataTypeArray *>(arrayType.get());
        if (!arrayDataType) {
            AddError("Internal error: invalid array type");
            return nullptr;
        }

        return arrayDataType->itemType;
    }

    bool SemanticAnalyzer::TypesEqual(const std::shared_ptr<DataType> &a, const std::shared_ptr<DataType> &b) {
        if (!a || !b) return false;
        if (a->Type() != b->Type()) return false;

        if (a->Type() == TypeDataType::Array) {
            const auto *arrA = dynamic_cast<DataTypeArray *>(a.get());
            const auto *arrB = dynamic_cast<DataTypeArray *>(b.get());
            if (!arrA || !arrB) return false;
            return TypesEqual(arrA->itemType, arrB->itemType);
        }

        return true;
    }

    bool SemanticAnalyzer::TypesCompatible(const std::shared_ptr<DataType> &expected,
                                           const std::shared_ptr<DataType> &actual) {
        return TypesEqual(expected, actual);
    }

    std::shared_ptr<DataType> SemanticAnalyzer::GetResultTypeForInfixOperation(
        const std::string &operator_,
        const std::shared_ptr<DataType> &left,
        const std::shared_ptr<DataType> &right) const {
        if (operator_ == "+" || operator_ == "-" || operator_ == "*" || operator_ == "/" || operator_ == "%") {
            if (left->Type() == TypeDataType::String || right->Type() == TypeDataType::String) {
                if (operator_ == "+") {
                    return GetStringType();
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
        const std::string &operator_,
        const std::shared_ptr<DataType> &operand) const {
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

    std::shared_ptr<DataType> SemanticAnalyzer::AnalyzeBuiltinCall(const std::string &functionName,
                                                                   const std::vector<std::shared_ptr<ExpressionNode> > &
                                                                   args) {
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
            const auto argType = AnalyzeExpression(args[0]);
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
            return GetStringType();
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
            if (functionName == "string") return GetStringType();
            if (functionName == "bool") return BOOL;
            if (functionName == "char") return GetCharType();
        }

        AddError("Unknown built-in function '" + functionName + "'");
        return nullptr;
    }

    void SemanticAnalyzer::AddError(const std::string &message, const std::string &location) const {
        errors.emplace_back(message, location);
    }

    bool SemanticAnalyzer::AnalyzeIfStatement(const std::shared_ptr<IfStatement> &ifStmt) {
        if (!ifStmt) {
            AddError("If statement is null");
            return false;
        }

        bool success = true;

        if (ifStmt->condition) {
            auto conditionType = AnalyzeExpression(ifStmt->condition);
            if (!conditionType) {
                AddError("Could not determine type of if condition");
                success = false;
            } else if (conditionType->Type() != TypeDataType::Bool) {
                AddError("If condition must be boolean, got " + conditionType->String());
                success = false;
            }
        } else {
            AddError("If statement missing condition");
            success = false;
        }

        if (ifStmt->thenStatement) {
            if (auto blockStmt = std::dynamic_pointer_cast<BlockStatement>(ifStmt->thenStatement)) {
                symbolTable.EnterScope();
                for (const auto &stmt: blockStmt->statements) {
                    success &= AnalyzeStatement(stmt);
                }
                symbolTable.ExitScope();
            } else {
                success &= AnalyzeStatement(ifStmt->thenStatement);
            }
        }

        if (ifStmt->elseStatement) {
            if (auto blockStmt = std::dynamic_pointer_cast<BlockStatement>(ifStmt->elseStatement)) {
                symbolTable.EnterScope();
                for (const auto &stmt: blockStmt->statements) {
                    success &= AnalyzeStatement(stmt);
                }
                symbolTable.ExitScope();
            } else if (auto nestedIfStmt = std::dynamic_pointer_cast<IfStatement>(ifStmt->elseStatement)) {
                success &= AnalyzeIfStatement(nestedIfStmt);
            } else {
                success &= AnalyzeStatement(ifStmt->elseStatement);
            }
        }

        return success;
    }

    bool SemanticAnalyzer::AnalyzeWhileStatement(const std::shared_ptr<WhileStatement> &whileStmt) {
        if (!whileStmt) {
            AddError("While statement is null");
            return false;
        }

        bool success = true;

        if (whileStmt->condition) {
            auto conditionType = AnalyzeExpression(whileStmt->condition);
            if (!conditionType) {
                AddError("Could not determine type of while condition");
                success = false;
            } else if (conditionType->Type() != TypeDataType::Bool) {
                AddError("While condition must be boolean, got " + conditionType->String());
                success = false;
            }
        } else {
            AddError("While statement missing condition");
            success = false;
        }

        if (whileStmt->stmt) {
            if (auto blockStmt = std::dynamic_pointer_cast<BlockStatement>(whileStmt->stmt)) {
                symbolTable.EnterScope();
                for (const auto &stmt: blockStmt->statements) {
                    success &= AnalyzeStatement(stmt);
                }
                symbolTable.ExitScope();
            } else {
                success &= AnalyzeStatement(whileStmt->stmt);
            }
        }

        return success;
    }

    bool SemanticAnalyzer::AnalyzeForStatement(const std::shared_ptr<ForStatement> &forStmt) {
        if (!forStmt) {
            AddError("For statement is null");
            return false;
        }

        bool success = true;

        symbolTable.EnterScope();

        if (forStmt->letStatement) {
            success &= AnalyzeLetStatement(forStmt->letStatement);
        } else {
            AddError("For statement missing initialization");
            success = false;
        }

        if (forStmt->condition) {
            auto conditionType = AnalyzeExpression(forStmt->condition);
            if (!conditionType) {
                AddError("Could not determine type of for condition");
                success = false;
            } else if (conditionType->Type() != TypeDataType::Bool) {
                AddError("For condition must be boolean, got " + conditionType->String());
                success = false;
            }
        } else {
            AddError("For statement missing condition");
            success = false;
        }

        if (forStmt->postStatement) {
            success &= AnalyzeStatement(forStmt->postStatement);
        } else {
            AddError("For statement missing post-statement");
            success = false;
        }

        if (forStmt->stmt) {
            if (auto blockStmt = std::dynamic_pointer_cast<BlockStatement>(forStmt->stmt)) {
                for (const auto &s: blockStmt->statements) {
                    success &= AnalyzeStatement(s);
                }
            } else {
                success &= AnalyzeStatement(forStmt->stmt);
            }
        }

        symbolTable.ExitScope();

        return success;
    }

    bool SemanticAnalyzer::AnalyzeExpressionStatement(const std::shared_ptr<ExpressionStatement> &exprStmt) {
        if (!exprStmt) {
            AddError("Expression statement is null");
            return false;
        }

        if (!exprStmt->expression) {
            AddError("Expression statement has no expression");
            return false;
        }

        auto exprType = AnalyzeExpression(exprStmt->expression);
        if (!exprType) {
            AddError("Could not determine type of expression in expression statement");
            return false;
        }

        return true;
    }

    bool SemanticAnalyzer::AnalyzeAssignStatement(const std::shared_ptr<AssignStatement> &assign) {
        if (!assign) {
            AddError("Assign statement is null");
            return false;
        }

        if (!assign->target) {
            AddError("Assign statement has no target");
            return false;
        }

        if (!assign->expression) {
            AddError("Assign statement has no expression");
            return false;
        }

        auto exprType = AnalyzeExpression(assign->expression);
        if (!exprType) {
            AddError("Could not determine type of expression in assignment");
            return false;
        }

        if (auto ident = std::dynamic_pointer_cast<Identifier>(assign->target)) {
            auto varType = symbolTable.LookupVariable(ident->TokenLiteral());
            if (!varType) {
                AddError("Cannot assign to undeclared variable '" + ident->TokenLiteral() + "'");
                return false;
            }
            if (!TypesCompatible(varType, exprType)) {
                AddError("Type mismatch in assignment: variable '" + ident->TokenLiteral() +
                         "' has type " + varType->String() + " but assigned " + exprType->String());
                return false;
            }
            return true;
        }

        if (auto indexExpr = std::dynamic_pointer_cast<IndexExpression>(assign->target)) {
            auto arrayType = AnalyzeExpression(indexExpr->left);
            auto idxType = AnalyzeExpression(indexExpr->index);

            if (!arrayType || !idxType) {
                return false;
            }

            if (arrayType->Type() != TypeDataType::Array) {
                AddError("Index assignment requires array type");
                return false;
            }

            if (idxType->Type() != TypeDataType::Int) {
                AddError("Array index must be int");
                return false;
            }

            auto *arrayDataType = dynamic_cast<DataTypeArray *>(arrayType.get());
            if (!arrayDataType) {
                AddError("Internal error: invalid array type");
                return false;
            }

            if (!TypesCompatible(arrayDataType->itemType, exprType)) {
                AddError("Type mismatch in array element assignment: expected " +
                         arrayDataType->itemType->String() + " but got " + exprType->String());
                return false;
            }

            return true;
        }

        AddError("Unsupported assignment target");
        return false;
    }
}
