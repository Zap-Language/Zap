#ifndef ZAP_SYMBOLTABLE_H
#define ZAP_SYMBOLTABLE_H

#include <stack>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "Scope.h"
#include "../DataType.h"
#include "../FuncStatement.h"

namespace ast {
    enum class SymbolType {
        VARIABLE,
        FUNCTION,
        PARAMETER
    };

    struct Symbol {
        std::string name;
        std::shared_ptr<DataType> type;
        SymbolType symbolType;
        size_t scopeDepth;
        std::string scopeName;
        
        Symbol(std::string name, std::shared_ptr<DataType> type, SymbolType symbolType, size_t scopeDepth, std::string scopeName = "")
            : name(std::move(name)), type(std::move(type)), symbolType(symbolType), scopeDepth(scopeDepth), scopeName(std::move(scopeName)) {}
        
        std::string ToString() const;
    };

    class SymbolTable {
    public:
        SymbolTable();
        ~SymbolTable() = default;

        void EnterScope();
        void ExitScope();
        std::shared_ptr<Scope> GetCurrentScope() const;
        size_t GetCurrentDepth() const;

        bool DeclareVariable(const std::string& name, std::shared_ptr<DataType> type);
        std::shared_ptr<DataType> LookupVariable(const std::string& name) const;
        bool VariableExists(const std::string& name) const;

        bool DeclareFunction(const std::string& name, std::shared_ptr<FuncStatement> func);
        std::shared_ptr<FuncStatement> LookupFunction(const std::string& name) const;
        bool FunctionExists(const std::string& name) const;

        bool DeclareParameter(const std::string& name, std::shared_ptr<DataType> type);

        void RegisterBuiltinFunctions();
        bool IsBuiltinFunction(const std::string& name) const;

        struct SymbolError {
            std::string message;
            std::string symbolName;
            size_t scopeDepth;
            
            SymbolError(std::string message, std::string symbolName, size_t scopeDepth)
                : message(std::move(message)), symbolName(std::move(symbolName)), scopeDepth(scopeDepth) {}
        };

        const std::vector<SymbolError>& GetErrors() const;
        void AddError(const std::string& message, const std::string& symbolName = "");
        void ClearErrors();

        std::string ToString() const;
        std::vector<Symbol> GetAllSymbols() const;

        const std::unordered_map<std::string, std::shared_ptr<FuncStatement>>& GetAllFunctions() const;
        const std::vector<Symbol>& GetAllStoredSymbols() const;
        void SetCurrentScopeName(const std::string& name);
        const std::string& GetCurrentScopeName() const;

    private:
        std::stack<std::shared_ptr<Scope>> scopes;
        std::vector<std::string> builtinFunctions;
        std::vector<SymbolError> errors;

        std::unordered_map<std::string, std::shared_ptr<FuncStatement>> allFunctions;
        std::vector<Symbol> allSymbols;
        std::string currentScopeName;

        void initializeBuiltins();
    };
}

#endif //ZAP_SYMBOLTABLE_H