#include "SymbolTable.h"
#include "../DataTypeInt.h"
#include "../DataTypeFloat.h"
#include "../DataTypeString.h"
#include "../DataTypeBool.h"
#include "../DataTypeChar.h"
#include "../DataTypeVoid.h"
#include <sstream>
#include <algorithm>
#include <unordered_map>

namespace ast {
    std::string Symbol::ToString() const {
        std::ostringstream oss;
        oss << name << " : " << (type ? type->String() : "null");
        oss << " (" << (symbolType == SymbolType::VARIABLE ? "var" :
                        symbolType == SymbolType::FUNCTION ? "func" : "param");
        oss << ", depth: " << scopeDepth << ")";
        return oss.str();
    }

    SymbolTable::SymbolTable() {
        EnterScope();
        initializeBuiltins();
    }

    void SymbolTable::EnterScope() {
        auto currentScope = scopes.empty() ? nullptr : scopes.top();
        scopes.push(std::make_shared<Scope>(currentScope));
    }

    void SymbolTable::ExitScope() {
        if (!scopes.empty()) {
            scopes.pop();
        }
    }

    std::shared_ptr<Scope> SymbolTable::GetCurrentScope() const {
        return scopes.empty() ? nullptr : scopes.top();
    }

    size_t SymbolTable::GetCurrentDepth() const {
        return scopes.size() - 1;
    }

    bool SymbolTable::DeclareVariable(const std::string& name, std::shared_ptr<DataType> type) {
        auto currentScope = GetCurrentScope();
        if (!currentScope) {
            AddError("No current scope available for variable declaration", name);
            return false;
        }

        if (!currentScope->DeclareVariable(name, type)) {
            AddError("Variable '" + name + "' already declared in current scope", name);
            return false;
        }
        allSymbols.emplace_back(name, type, SymbolType::VARIABLE, GetCurrentDepth(), currentScopeName);

        return true;
    }

    std::shared_ptr<DataType> SymbolTable::LookupVariable(const std::string& name) const {
        auto currentScope = GetCurrentScope();
        if (!currentScope) {
            return nullptr;
        }

        return currentScope->LookupVariable(name);
    }

    bool SymbolTable::VariableExists(const std::string& name) const {
        return LookupVariable(name) != nullptr;
    }

    bool SymbolTable::DeclareFunction(const std::string& name, std::shared_ptr<FuncStatement> func) {
        auto currentScope = GetCurrentScope();
        if (!currentScope) {
            AddError("No current scope available for function declaration", name);
            return false;
        }

        if (!currentScope->DeclareFunction(name, func)) {
            AddError("Function '" + name + "' already declared in current scope", name);
            return false;
        }
        std::string fullName = currentScopeName.empty() ? name : currentScopeName + "::" + name;
        allFunctions[fullName] = func;
        allSymbols.emplace_back(name, func->returnType, SymbolType::FUNCTION, GetCurrentDepth(), currentScopeName);

        return true;
    }

    std::shared_ptr<FuncStatement> SymbolTable::LookupFunction(const std::string& name) const {
        auto currentScope = GetCurrentScope();
        if (!currentScope) {
            return nullptr;
        }

        return currentScope->LookupFunction(name);
    }

    bool SymbolTable::FunctionExists(const std::string& name) const {
        return LookupFunction(name) != nullptr || IsBuiltinFunction(name);
    }

    bool SymbolTable::DeclareParameter(const std::string& name, std::shared_ptr<DataType> type) {
        auto currentScope = GetCurrentScope();
        if (!currentScope) {
            AddError("No current scope available for parameter declaration", name);
            return false;
        }

        if (!currentScope->DeclareVariable(name, type)) {
            AddError("Parameter '" + name + "' already declared in current scope", name);
            return false;
        }
        allSymbols.emplace_back(name, type, SymbolType::PARAMETER, GetCurrentDepth(), currentScopeName);

        return true;
    }

    void SymbolTable::RegisterBuiltinFunctions() {
        initializeBuiltins();
    }

    bool SymbolTable::IsBuiltinFunction(const std::string& name) const {
        return std::find(builtinFunctions.begin(), builtinFunctions.end(), name) != builtinFunctions.end();
    }

    const std::vector<SymbolTable::SymbolError>& SymbolTable::GetErrors() const {
        return errors;
    }

    void SymbolTable::AddError(const std::string& message, const std::string& symbolName) {
        errors.emplace_back(message, symbolName, GetCurrentDepth());
    }

    void SymbolTable::ClearErrors() {
        errors.clear();
    }

    std::string SymbolTable::ToString() const {
        std::ostringstream oss;
        oss << "All Functions (" << allFunctions.size() << ")\n";
        for (const auto& [name, func]: allFunctions) {
            oss << "  " << name << " : " << func->returnType->String() << "(";
            if (func->arguments) {
                for (size_t i = 0; i < func->arguments->arguments.size(); ++i) {
                    if (i > 0) oss << ", ";
                    oss << func->arguments->arguments[i]->type->String();
                }
            }
            oss << ")\n";
        }
        oss << "\nAll Symbols (" << allSymbols.size() << ")\n";
        std::unordered_map<std::string, std::vector<const Symbol *>> symbolsByScope;

        for (const auto& symbol: allSymbols) {
            std::string scopeKey = symbol.scopeName.empty() ? "<global>" : symbol.scopeName;
            symbolsByScope[scopeKey].push_back(&symbol);
        }

        for (const auto& [scopeName, symbols]: symbolsByScope) {
            oss << "\n  Scope: " << scopeName << "\n";
            for (const auto *symbol: symbols) {
                oss << "    " << symbol->ToString() << "\n";
            }
        }

        if (!errors.empty()) {
            oss << "\nErrors\n";
            for (const auto& error: errors) {
                oss << "  [" << error.symbolName << "] " << error.message << " (depth: " << error.scopeDepth << ")\n";
            }
        }

        return oss.str();
    }

    std::vector<Symbol> SymbolTable::GetAllSymbols() const {
        std::vector<Symbol> allSymbols;

        std::stack<std::shared_ptr<Scope>> tempStack = scopes;
        std::vector<std::shared_ptr<Scope>> scopeList;

        while (!tempStack.empty()) {
            scopeList.push_back(tempStack.top());
            tempStack.pop();
        }

        std::reverse(scopeList.begin(), scopeList.end());

        for (size_t depth = 0; depth < scopeList.size(); ++depth) {
            const auto& scope = scopeList[depth];
            for (const auto& [name, type]: scope->GetVariables()) {
                allSymbols.emplace_back(name, type, SymbolType::VARIABLE, depth);
            }
            for (const auto& [name, func]: scope->GetFunctions()) {
                auto funcType = func->returnType;
                allSymbols.emplace_back(name, funcType, SymbolType::FUNCTION, depth);
            }
        }

        return allSymbols;
    }

    void SymbolTable::initializeBuiltins() {
        builtinFunctions = {
                "print",
                "len",
                "read"
                "int",
                "float",
                "string"
                "bool",
                "char"
        };
    }

    const std::unordered_map<std::string, std::shared_ptr<FuncStatement>>& SymbolTable::GetAllFunctions() const {
        return allFunctions;
    }

    const std::vector<Symbol>& SymbolTable::GetAllStoredSymbols() const {
        return allSymbols;
    }

    void SymbolTable::SetCurrentScopeName(const std::string& name) {
        currentScopeName = name;
    }

    const std::string& SymbolTable::GetCurrentScopeName() const {
        return currentScopeName;
    }
}