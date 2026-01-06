#include <fstream>
#include <iostream>
#include "vm.h"
#include "jit.h"

using namespace jit;

int main(int argc, const char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: vm <bytecode_file>" << std::endl;
        return 1;
    }

    std::ifstream ifs(argv[1], std::ios::binary);
    if (!ifs.is_open()) return 1;

    auto chunk = bytecode::BytecodeChunk::Deserialize(ifs);

    VM vm;
    JITCompiler jit(&vm);
    vm.setJITCompiler(&jit);

    vm.loadChunk(std::move(chunk));
    vm.run();

    return 0;
}
