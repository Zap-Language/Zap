//

#ifndef ZAP_SYMBOL_TABLE_H
#define ZAP_SYMBOL_TABLE_H

#pragma once

#include "value_type.h"

#include <string>
#include <unordered_map>
#include <optional>
#include <memory>
#include <vector>

namespace bytecode {

enum class SymbolScope {
    GLOBAL,    
    LOCAL,     
    FUNCTION,  
    BUILTIN,   
};

struct Symbol {
    std::string name;      
    ValueType type;        
    SymbolScope scope;     
    uint32_t index;        
    bool isConstant;       

    Symbol() = default;
    Symbol(std::string n, ValueType t, SymbolScope s, uint32_t i, bool c = false)
        : name(std::move(n)), type(t), scope(s), index(i), isConstant(c) {}
};

class SymbolTable {
public:
    SymbolTable();
    explicit SymbolTable(std::shared_ptr<SymbolTable> parent);
    Symbol Define(const std::string& name, ValueType type, bool isConstant = false);
    Symbol DefineFunction(const std::string& name, uint32_t funcIndex, ValueType returnType);
    Symbol DefineBuiltin(const std::string& name, uint8_t builtinIndex);
    std::optional<Symbol> Resolve(const std::string& name) const;
    std::optional<Symbol> ResolveLocal(const std::string& name) const;
    uint32_t LocalCount() const { return _localCount; }
    uint32_t GlobalCount() const { return _globalCount; }
    std::shared_ptr<SymbolTable> Parent() const { return _parent; }
    bool IsGlobal() const { return _parent == nullptr; }

private:
    std::shared_ptr<SymbolTable> _parent;
    std::unordered_map<std::string, Symbol> _symbols;
    uint32_t _localCount;
    uint32_t _globalCount; 
};

}

#endif