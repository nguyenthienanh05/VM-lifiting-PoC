#pragma once

#include <cstdint>
#include <string>
#include <vector>


struct Instruction {
    size_t n_operands;
    std::string name;
    uint8_t opcode;
    uint8_t ins_size;
    uint32_t vm_rip;
    std::vector<uint8_t> operands = {};

    Instruction(size_t n_ops, const std::string& name, uint8_t opcode, uint8_t ins_size)
        : n_operands(n_ops), name(std::move(name)), opcode(opcode), ins_size(ins_size), vm_rip(0) {}
};

class Disassembler {
public:
    static std::vector<Instruction> disassemble(const std::vector<uint8_t>& bytecode, uint32_t start_address = 0);
};
