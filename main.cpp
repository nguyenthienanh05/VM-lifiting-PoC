#include <fstream>
#include "src/disassembler.h"
#include "src/lifter.h"
#include <iomanip>
#include <iostream>

int main(int argc, char* argv[], char* envp[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <bytecode_file>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1], std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file: " << argv[1] << std::endl;
        return 1;
    }

    std::vector<uint8_t> bytecode((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    try {
        auto instructions = Disassembler::disassemble(bytecode);
        // for (const auto& ins : instructions) {
        //     std::cout << "0x" << std::hex << ins.vm_rip - 1 << ": " << ins.name;
        //     for (const auto& op : ins.operands) {
        //         std::cout << " 0x" << std::hex << static_cast<int>(op);
        //     }
        //     std::cout << std::dec << std::endl;
        // }
        Lifter lifter;
        lifter.lift(instructions);
        lifter.optimize();
        // lifter.dump_output();
        lifter.saveOutput();
    } catch (const std::exception& e) {
        std::cerr << "Lifting error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}