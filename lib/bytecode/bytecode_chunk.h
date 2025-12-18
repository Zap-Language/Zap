#pragma once

#include "opcodes.h"
#include "value_type.h"

#include <vector>
#include <string>
#include <cstdint>
#include <cstring>

namespace bytecode {

struct FunctionInfo {
    std::string name;                  
    uint32_t codeOffset;               
    uint32_t codeLength;               
    uint8_t paramCount;                
    uint8_t localCount;                
    ValueType returnType;              
    std::vector<ValueType> paramTypes; 
};

class BytecodeChunk {
public:
    BytecodeChunk() = default;

   

    const std::vector<uint8_t>& Code() const { return _code; }
    const std::vector<std::string>& Strings() const { return _strings; }
    const std::vector<FunctionInfo>& Functions() const { return _functions; }

    size_t CurrentOffset() const { return _code.size(); }

   

    void EmitOpCode(OpCode op) {
        _code.push_back(static_cast<uint8_t>(op));
    }

    void EmitByte(uint8_t byte) {
        _code.push_back(byte);
    }

    void EmitBytes(const uint8_t* data, size_t size) {
        _code.insert(_code.end(), data, data + size);
    }

    void EmitUint32(uint32_t value) {
        _code.push_back(static_cast<uint8_t>(value & 0xFF));
        _code.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
        _code.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
        _code.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
    }

    void EmitInt64(int64_t value) {
        for (int i = 0; i < 8; ++i) {
            _code.push_back(static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
        }
    }

    void EmitDouble(double value) {
        uint64_t bits;
        std::memcpy(&bits, &value, sizeof(double));
        for (int i = 0; i < 8; ++i) {
            _code.push_back(static_cast<uint8_t>((bits >> (i * 8)) & 0xFF));
        }
    }

   

    void PatchUint32(size_t offset, uint32_t value) {
        _code[offset]     = static_cast<uint8_t>(value & 0xFF);
        _code[offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
        _code[offset + 2] = static_cast<uint8_t>((value >> 16) & 0xFF);
        _code[offset + 3] = static_cast<uint8_t>((value >> 24) & 0xFF);
    }

   

    uint32_t AddString(const std::string& str) {
        _strings.push_back(str);
        return static_cast<uint32_t>(_strings.size() - 1);
    }

    uint32_t AddFunction(const FunctionInfo& func) {
        _functions.push_back(func);
        return static_cast<uint32_t>(_functions.size() - 1);
    }

    FunctionInfo& GetFunction(uint32_t index) {
        return _functions[index];
    }

   

    uint8_t ReadByte(size_t offset) const {
        return _code[offset];
    }

    uint32_t ReadUint32(size_t offset) const {
        return static_cast<uint32_t>(_code[offset]) |
               (static_cast<uint32_t>(_code[offset + 1]) << 8) |
               (static_cast<uint32_t>(_code[offset + 2]) << 16) |
               (static_cast<uint32_t>(_code[offset + 3]) << 24);
    }

    int64_t ReadInt64(size_t offset) const {
        int64_t value = 0;
        for (int i = 0; i < 8; ++i) {
            value |= static_cast<int64_t>(_code[offset + i]) << (i * 8);
        }
        return value;
    }

    double ReadDouble(size_t offset) const {
        uint64_t bits = 0;
        for (int i = 0; i < 8; ++i) {
            bits |= static_cast<uint64_t>(_code[offset + i]) << (i * 8);
        }
        double value;
        std::memcpy(&value, &bits, sizeof(double));
        return value;
    }

private:
    std::vector<uint8_t> _code;          
    std::vector<std::string> _strings;   
    std::vector<FunctionInfo> _functions;
};

}