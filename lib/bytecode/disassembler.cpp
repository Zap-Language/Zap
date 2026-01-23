#include "disassembler.h"
#include <iomanip>

namespace bytecode {

void Disassembler::Disassemble(const BytecodeChunk& chunk, std::ostream& out) {
    out << "╔══════════════════════════════════════════════════════════╗\n";
    out << "║                   BYTECODE DISASSEMBLY                   ║\n";
    out << "╚══════════════════════════════════════════════════════════╝\n\n";

   
    if (!chunk.Strings().empty()) {
        out << "┌─── String Pool ───────────────────────────────────────────\n";
        for (size_t i = 0; i < chunk.Strings().size(); ++i) {
            out << "│ [" << i << "] \"" << chunk.Strings()[i] << "\"\n";
        }
        out << "└───────────────────────────────────────────────────────────\n\n";
    }

   
    if (!chunk.Structs().empty()) {
        out << "┌─── Structs ───────────────────────────────────────────────\n";
        for (size_t i = 0; i < chunk.Structs().size(); ++i) {
            const auto& s = chunk.Structs()[i];
            out << "│ [" << i << "] " << s.name << " (fields: " << s.fieldTypes.size() << ")\n";
        }
        out << "└───────────────────────────────────────────────────────────\n\n";
    }

    if (!chunk.Functions().empty()) {
        out << "┌─── Functions ─────────────────────────────────────────────\n";
        for (size_t i = 0; i < chunk.Functions().size(); ++i) {
            const auto& f = chunk.Functions()[i];
            out << "│ [" << i << "] " << f.name
                << " (params: " << static_cast<int>(f.paramCount)
                << ", locals: " << static_cast<int>(f.localCount)
                << ", offset: " << f.codeOffset
                << ", len: " << f.codeLength << ")\n";
        }
        out << "└───────────────────────────────────────────────────────────\n\n";
    }

   
    out << "┌─── Instructions ─────────────────────────────────────────\n";
    size_t offset = 0;
    while (offset < chunk.Code().size()) {
        out << "│ ";
        offset = DisassembleInstruction(chunk, offset, out);
    }
    out << "└───────────────────────────────────────────────────────────\n";
}

size_t Disassembler::DisassembleInstruction(const BytecodeChunk& chunk,
                                             size_t offset,
                                             std::ostream& out) {
   
    out << std::setfill('0') << std::setw(6) << offset << std::setfill(' ') << "  ";

    auto op = static_cast<OpCode>(chunk.ReadByte(offset));
    offset++;

   
    out << std::left << std::setw(16) << OpCodeToString(op) << std::right;

    switch (op) {
        case OpCode::PUSH_INT: {
            int64_t value = chunk.ReadInt64(offset);
            out << value;
            offset += 8;
            break;
        }

        case OpCode::PUSH_FLOAT: {
            double value = chunk.ReadDouble(offset);
            out << value;
            offset += 8;
            break;
        }

        case OpCode::PUSH_BOOL: {
            bool value = chunk.ReadByte(offset) != 0;
            out << (value ? "true" : "false");
            offset += 1;
            break;
        }

        case OpCode::PUSH_CHAR: {
            char value = static_cast<char>(chunk.ReadByte(offset));
            out << "'" << value << "' (" << static_cast<int>(value) << ")";
            offset += 1;
            break;
        }

        case OpCode::PUSH_STRING: {
            uint32_t index = chunk.ReadUint32(offset);
            out << "@" << index;
            if (index < chunk.Strings().size()) {
                out << " (\"" << chunk.Strings()[index] << "\")";
            }
            offset += 4;
            break;
        }

        case OpCode::PUSH_FUNC: {
            uint32_t funcIndex = chunk.ReadUint32(offset);
            out << "func[" << funcIndex << "]";
            if (funcIndex < chunk.Functions().size()) {
                out << " (" << chunk.Functions()[funcIndex].name << ")";
            }
            offset += 4;
            break;
        }

        case OpCode::LOAD_LOCAL:
        case OpCode::STORE_LOCAL: {
            uint32_t index = chunk.ReadUint32(offset);
            out << "#" << index;
            offset += 4;
            break;
        }

        case OpCode::LOAD_GLOBAL:
        case OpCode::STORE_GLOBAL: {
            uint32_t index = chunk.ReadUint32(offset);
            out << "g#" << index;
            offset += 4;
            break;
        }

        case OpCode::JMP:
        case OpCode::JMP_IF_FALSE:
        case OpCode::JMP_IF_TRUE: {
            uint32_t target = chunk.ReadUint32(offset);
            out << "-> " << std::setfill('0') << std::setw(6) << target << std::setfill(' ');
            offset += 4;
            break;
        }

        case OpCode::CALL: {
            uint32_t funcIndex = chunk.ReadUint32(offset);
            uint8_t argCount = chunk.ReadByte(offset + 4);
            out << "func[" << funcIndex << "]";
            if (funcIndex < chunk.Functions().size()) {
                out << " (" << chunk.Functions()[funcIndex].name << ")";
            }
            out << " args=" << static_cast<int>(argCount);
            offset += 5;
            break;
        }

        case OpCode::CALL_VALUE: {
            uint8_t argCount = chunk.ReadByte(offset);
            out << "<value> args=" << static_cast<int>(argCount);
            offset += 1;
            break;
        }

        case OpCode::CALL_BUILTIN: {
            uint8_t builtinId = chunk.ReadByte(offset);
            uint8_t argCount = chunk.ReadByte(offset + 1);

           
            std::string builtinName;
            switch (static_cast<BuiltinFunction>(builtinId)) {
                case BuiltinFunction::PRINT:       builtinName = "print"; break;
                case BuiltinFunction::LEN:         builtinName = "len"; break;
                case BuiltinFunction::READ:        builtinName = "read"; break;
                case BuiltinFunction::CAST_INT:    builtinName = "int"; break;
                case BuiltinFunction::CAST_FLOAT:  builtinName = "float"; break;
                case BuiltinFunction::CAST_BOOL:   builtinName = "bool"; break;
                case BuiltinFunction::CAST_CHAR:   builtinName = "char"; break;
                case BuiltinFunction::CAST_STRING: builtinName = "string"; break;
                default: builtinName = "unknown"; break;
            }

            out << builtinName << " args=" << static_cast<int>(argCount);
            offset += 2;
            break;
        }

        case OpCode::NEW_ARRAY: {
            uint8_t elemType = chunk.ReadByte(offset);
            uint32_t size = chunk.ReadUint32(offset + 1);
            out << "type=" << ValueTypeToString(static_cast<ValueType>(elemType))
                << " size=" << size;
            offset += 5;
            break;
        }

        case OpCode::NEW_STRUCT: {
            uint32_t structId = chunk.ReadUint32(offset);
            uint8_t argCount = chunk.ReadByte(offset + 4);
            out << "struct[" << structId << "]";
            if (structId < chunk.Structs().size()) {
                out << " (" << chunk.Structs()[structId].name << ")";
            }
            out << " args=" << static_cast<int>(argCount);
            offset += 5;
            break;
        }

        case OpCode::GET_FIELD:
        case OpCode::SET_FIELD: {
            uint32_t structId = chunk.ReadUint32(offset);
            uint8_t fieldIndex = chunk.ReadByte(offset + 4);
            out << "struct[" << structId << "]";
            if (structId < chunk.Structs().size()) {
                out << " (" << chunk.Structs()[structId].name << ")";
            }
            out << " field=" << static_cast<int>(fieldIndex);
            offset += 5;
            break;
        }

       
        case OpCode::ADD:
        case OpCode::SUB:
        case OpCode::MUL:
        case OpCode::DIV:
        case OpCode::MOD:
        case OpCode::POW:
        case OpCode::NEG:
        case OpCode::AND:
        case OpCode::OR:
        case OpCode::NOT:
        case OpCode::CMP_EQ:
        case OpCode::CMP_NE:
        case OpCode::CMP_LT:
        case OpCode::CMP_GT:
        case OpCode::CMP_LE:
        case OpCode::CMP_GE:
        case OpCode::RETURN:
        case OpCode::RETURN_VOID:
        case OpCode::ARRAY_GET:
        case OpCode::ARRAY_SET:
        case OpCode::ARRAY_LEN:
        case OpCode::CAST_INT:
        case OpCode::CAST_FLOAT:
        case OpCode::CAST_BOOL:
        case OpCode::CAST_CHAR:
        case OpCode::CAST_STRING:
        case OpCode::POP:
        case OpCode::DUP:
        case OpCode::PUSH_NULL:
        case OpCode::NOP:
        case OpCode::HALT:
           
            break;

        default:
            out << "(unknown opcode: " << static_cast<int>(op) << ")";
            break;
    }

    out << "\n";
    return offset;
}

}