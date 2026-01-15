#ifndef ZAP_SCOPE_H
#define ZAP_SCOPE_H

#include <string>
#include <unordered_map>
#include <memory>
#include "../DataType.h"
#include "../FuncStatement.h"

namespace ast {
    class Scope {
    public:
        explicit Scope(std::shared_ptr<Scope> parent = nullptr);
        ~Scope() = default;

        bool DeclareVariable(const std::string& name, std::shared_ptr<DataType> type);
        std::shared_ptr<DataType> LookupVariable(const std::string& name) const;
        bool VariableExists(const std::string& name) const;
        bool VariableExistsInCurrentScope(const std::string& name) const;

        bool DeclareFunction(const std::string& name, std::shared_ptr<FuncStatement> func);
        std::shared_ptr<FuncStatement> LookupFunction(const std::string& name) const;
        bool FunctionExists(const std::string& name) const;
        bool FunctionExistsInCurrentScope(const std::string& name) const;

        std::shared_ptr<Scope> GetParent() const;
        const std::unordered_map<std::string, std::shared_ptr<DataType>>& GetVariables() const;
        const std::unordered_map<std::string, std::shared_ptr<FuncStatement>>& GetFunctions() const;

        std::string ToString() const;
        size_t GetDepth() const;

    private:
        std::unordered_map<std::string, std::shared_ptr<DataType>> variables;
        std::unordered_map<std::string, std::shared_ptr<FuncStatement>> functions;
        std::shared_ptr<Scope> parent;
    };
}

#endif //ZAP_SCOPE_H