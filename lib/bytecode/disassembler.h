//
// Created by Voice on 17.12.2025.
//

#ifndef ZAP_DISASSEMBLER_H
#define ZAP_DISASSEMBLER_H

#pragma once

#include "bytecode_chunk.h"
#include <ostream>
#include <string>

namespace bytecode {
    class Disassembler {
    public:
        static void Disassemble(const BytecodeChunk& chunk, std::ostream& out);
        static size_t DisassembleInstruction(const BytecodeChunk& chunk,
                                              size_t offset,
                                              std::ostream& out);
    };

}

#endif