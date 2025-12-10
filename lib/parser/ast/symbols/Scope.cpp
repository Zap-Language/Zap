#include "Scope.h"
#include <sstream>

namespace ast {
    Scope::Scope(std::shared_ptr<Scope> parent) : parent(std::move(parent)) {}

    bool Scope::DeclareVariable(const std::string& name, std::shared_ptr<DataType> type) {
        if (!type) {
            return false;
        }

        if (VariableExistsInCurrentScope(name)) {
            return false;
        }
        
        variables[name] = std::move(type);
        return true;
    }

    std::shared_ptr<DataType> Scope::LookupVariable(const std::string& name) const {
        auto it = variables.find(name);
        if (it != variables.end()) {
            return it->second;
        }
        if (parent) {
            return parent->LookupVariable(name);
        }
        
        return nullptr;
    }

    bool Scope::VariableExists(const std::string& name) const {
        return LookupVariable(name) != nullptr;
    }

    bool Scope::VariableExistsInCurrentScope(const std::string& name) const {
        return variables.find(name) != variables.end();
    }

    bool Scope::DeclareFunction(const std::string& name, std::shared_ptr<FuncStatement> func) {
        if (!func) {
            return false;
        }
        if (FunctionExistsInCurrentScope(name)) {
            return false;
        }
        
        functions[name] = std::move(func);
        return true;
    }

    std::shared_ptr<FuncStatement> Scope::LookupFunction(const std::string& name) const {
        auto it = functions.find(name);
        if (it != functions.end()) {
            return it->second;
        }
        if (parent) {
            return parent->LookupFunction(name);
        }
        
        return nullptr;
    }

    bool Scope::FunctionExists(const std::string& name) const {
        return LookupFunction(name) != nullptr;
    }

    bool Scope::FunctionExistsInCurrentScope(const std::string& name) const {
        return functions.find(name) != functions.end();
    }

    std::shared_ptr<Scope> Scope::GetParent() const {
        return parent;
    }

    const std::unordered_map<std::string, std::shared_ptr<DataType>>& Scope::GetVariables() const {
        return variables;
    }

    const std::unordered_map<std::string, std::shared_ptr<FuncStatement>>& Scope::GetFunctions() const {
        return functions;
    }

    std::string Scope::ToString() const {
        std::ostringstream oss;
        oss << "Scope (depth: " << GetDepth() << "):\n";
        
        oss << "  Variables:\n";
        for (const auto& [name, type] : variables) {
            oss << "    " << name << " : " << (type ? type->String() : "null") << "\n";
        }
        
        oss << "  Functions:\n";
        for (const auto& [name, func] : functions) {
            oss << "    " << name << " : " << (func ? func->returnType->String() : "null") << "\n";
        }
        
        return oss.str();
    }

    size_t Scope::GetDepth() const {
        size_t depth = 0;
        auto current = parent;
        while (current) {
            depth++;
            current = current->GetParent();
        }
        return depth;
    }
}