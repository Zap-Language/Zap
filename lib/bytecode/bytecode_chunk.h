#pragma once

#include "opcodes.h"
#include "value_type.h"

#include <vector>
#include <string>
#include <cstring>
#include <istream>
#include <memory>

namespace bytecode {

template<typename T>
void writeRaw(std::ostream& os, const T& value) {
    os.write(reinterpret_cast<const char*>(&value), sizeof(T));
}

template<typename T>
void readRaw(std::istream& is, T& value) {
    is.read(reinterpret_cast<char*>(&value), sizeof(T));
}

struct FunctionInfo {
    std::string name;                  
    uint32_t codeOffset{};
    uint32_t codeLength{};
    uint8_t paramCount{};
    uint8_t localCount{};
    ValueType returnType{};
    std::vector<ValueType> paramTypes; 
};

class BytecodeChunk {
public:
    BytecodeChunk() = default;

   

    [[nodiscard]] const std::vector<uint8_t>& Code() const { return _code; }
    [[nodiscard]] const std::vector<std::string>& Strings() const { return _strings; }
    [[nodiscard]] const std::vector<FunctionInfo>& Functions() const { return _functions; }

    [[nodiscard]] size_t CurrentOffset() const { return _code.size(); }

   

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

   

    [[nodiscard]] uint8_t ReadByte(size_t offset) const {
        return _code[offset];
    }

    [[nodiscard]] uint32_t ReadUint32(size_t offset) const {
        return static_cast<uint32_t>(_code[offset]) |
               (static_cast<uint32_t>(_code[offset + 1]) << 8) |
               (static_cast<uint32_t>(_code[offset + 2]) << 16) |
               (static_cast<uint32_t>(_code[offset + 3]) << 24);
    }

    [[nodiscard]] int64_t ReadInt64(size_t offset) const {
        int64_t value = 0;
        for (int i = 0; i < 8; ++i) {
            int64_t byte = (i == 7) 
                ? static_cast<int8_t>(_code[offset + i])
                : static_cast<int64_t>(_code[offset + i]);
            value |= byte << (i * 8);
        }
        return value;
    }

    [[nodiscard]] double ReadDouble(size_t offset) const {
        uint64_t bits = 0;
        for (int i = 0; i < 8; ++i) {
            bits |= static_cast<uint64_t>(_code[offset + i]) << (i * 8);
        }
        double value;
        std::memcpy(&value, &bits, sizeof(double));
        return value;
    }

    void Serialize(std::ostream& os) const {
        // 1. Заголовок (Magic Number), чтобы отличать наш файл
        uint32_t magic = 0x42434F44; // "BCOD"
        writeRaw(os, magic);

        // 2. Сериализация байт-кода (_code)
        const auto codeSize = static_cast<uint32_t>(_code.size());
        writeRaw(os, codeSize);
        os.write(reinterpret_cast<const char*>(_code.data()), codeSize);

        // 3. Сериализация строк (_strings)
        const auto stringCount = static_cast<uint32_t>(_strings.size());
        writeRaw(os, stringCount);
        for (const auto& s : _strings) {
            auto len = static_cast<uint32_t>(s.length());
            writeRaw(os, len);
            os.write(s.data(), len);
        }

        // 4. Сериализация информации о функциях (_functions)
        auto funcCount = static_cast<uint32_t>(_functions.size());
        writeRaw(os, funcCount);
        for (const auto& f : _functions) {
            // Имя функции
            auto nameLen = static_cast<uint32_t>(f.name.length());
            writeRaw(os, nameLen);
            os.write(f.name.data(), nameLen);

            // Основные поля
            writeRaw(os, f.codeOffset);
            writeRaw(os, f.codeLength);
            writeRaw(os, f.paramCount);
            writeRaw(os, f.localCount);
            writeRaw(os, f.returnType);

            // Вектор типов параметров (paramTypes)
            auto paramTypesSize = static_cast<uint32_t>(f.paramTypes.size());
            writeRaw(os, paramTypesSize);
            for (const auto& type : f.paramTypes) {
                writeRaw(os, type);
            }
        }
    }

    static std::unique_ptr<BytecodeChunk> Deserialize(std::istream& is) {
        auto chunk = std::make_unique<BytecodeChunk>();

        // 1. Проверка заголовка
        uint32_t magic;
        readRaw(is, magic);
        if (magic != 0x42434F44) {
            throw std::runtime_error("Invalid bytecode format (magic number mismatch)");
        }

        // 2. Читаем байт-код
        uint32_t codeSize;
        readRaw(is, codeSize);
        chunk->_code.resize(codeSize);
        is.read(reinterpret_cast<char*>(chunk->_code.data()), codeSize);

        // 3. Читаем строки
        uint32_t stringCount;
        readRaw(is, stringCount);
        chunk->_strings.reserve(stringCount);
        for (uint32_t i = 0; i < stringCount; ++i) {
            uint32_t len;
            readRaw(is, len);
            std::string s(len, '\0');
            is.read(&s[0], len);
            chunk->_strings.push_back(std::move(s));
        }

        // 4. Читаем функции
        uint32_t funcCount;
        readRaw(is, funcCount);
        chunk->_functions.reserve(funcCount);
        for (uint32_t i = 0; i < funcCount; ++i) {
            FunctionInfo f;

            // Имя
            uint32_t nameLen;
            readRaw(is, nameLen);
            f.name.resize(nameLen);
            is.read(&f.name[0], nameLen);

            // Поля
            readRaw(is, f.codeOffset);
            readRaw(is, f.codeLength);
            readRaw(is, f.paramCount);
            readRaw(is, f.localCount);
            readRaw(is, f.returnType);

            // Типы параметров
            uint32_t paramTypesSize;
            readRaw(is, paramTypesSize);
            f.paramTypes.resize(paramTypesSize);
            for (uint32_t j = 0; j < paramTypesSize; ++j) {
                readRaw(is, f.paramTypes[j]);
            }

            chunk->_functions.push_back(std::move(f));
        }

        return chunk;
    }

private:
    std::vector<uint8_t> _code;
    std::vector<std::string> _strings;
    std::vector<FunctionInfo> _functions;
};



}
