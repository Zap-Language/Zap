#include "symbol_table.h"

namespace bytecode {

    SymbolTable::SymbolTable()
        : _parent(nullptr)
        , _localCount(0)
        , _globalCount(0) {
    }

    SymbolTable::SymbolTable(std::shared_ptr<SymbolTable> parent)
        : _parent(std::move(parent))
        , _localCount(0)
        , _globalCount(0) {
    }

    Symbol SymbolTable::Define(const std::string& name, ValueType type, bool isConstant) {
        Symbol symbol;
        symbol.name = name;
        symbol.type = type;
        symbol.isConstant = isConstant;

        if (IsGlobal()) {
            symbol.scope = SymbolScope::GLOBAL;
            symbol.index = _globalCount++;
        } else {
            symbol.scope = SymbolScope::LOCAL;
            symbol.index = _localCount++;
        }

        _symbols[name] = symbol;
        return symbol;
    }

    Symbol SymbolTable::DefineFunction(const std::string& name, uint32_t funcIndex, ValueType returnType) {
        Symbol symbol;
        symbol.name = name;
        symbol.type = returnType;
        symbol.scope = SymbolScope::FUNCTION;
        symbol.index = funcIndex;
        symbol.isConstant = true;

        _symbols[name] = symbol;
        return symbol;
    }

    Symbol SymbolTable::DefineBuiltin(const std::string& name, uint8_t builtinIndex) {
        Symbol symbol;
        symbol.name = name;
        symbol.type = ValueType::VOID; 
        symbol.scope = SymbolScope::BUILTIN;
        symbol.index = builtinIndex;
        symbol.isConstant = true;

        _symbols[name] = symbol;
        return symbol;
    }

    std::optional<Symbol> SymbolTable::Resolve(const std::string& name) const {
        auto it = _symbols.find(name);
        if (it != _symbols.end()) {
            return it->second;
        }
        if (_parent != nullptr) {
            return _parent->Resolve(name);
        }
        return std::nullopt;
    }

    std::optional<Symbol> SymbolTable::ResolveLocal(const std::string& name) const {
        auto it = _symbols.find(name);
        if (it != _symbols.end()) {
            return it->second;
        }
        return std::nullopt;
    }

}