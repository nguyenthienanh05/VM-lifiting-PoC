#include "disassembler.h"

#include <cstring>
#include <fstream>
#include <stdexcept>
#include <unordered_map>

static const std::unordered_map<uint8_t, Instruction> OPCODES = {
    {0x20, {0, "VM_EXIT", 0x20, 1}},
    {0x21, {1, "PRINT", 0x21, 2}},
    {0x22, {0, "GETCHAR", 0x22, 1}},
    {0x23, {2, "MOV_IMM", 0x23, 3}},
    {0x24, {2, "MOV", 0x24, 3}},
    {0x25, {3, "AND", 0x25, 4}},
    {0x26, {3, "OR", 0x26, 4}},
    {0x27, {3, "XOR", 0x27, 4}},
    {0x28, {0, "POP_INP", 0x28, 1}}
};

std::vector<Instruction> Disassembler::disassemble(const std::vector<uint8_t>& bytecode, uint32_t start_address) {
    std::vector<Instruction> instructions;
    size_t vip = start_address;

    while (vip < bytecode.size()) {
        uint8_t opcode = bytecode[vip];
        auto it = OPCODES.find(opcode);
        if (it == OPCODES.end()) {
            throw std::runtime_error("Unknown opcode: " + std::to_string(opcode));
        }

        if (opcode == 0x20) {
            // VM_EXIT
            break;
        }
        vip++;
        Instruction ins = it->second;
        ins.vm_rip = vip;
        for (size_t i = 0; i < ins.n_operands; ++i){
            if (vip + i >= bytecode.size()) {
                throw std::runtime_error("Unexpected end of bytecode while reading operands.");
            }
            ins.operands.push_back(bytecode[vip + i]);
        };
        
        vip += ins.n_operands;
        instructions.push_back(ins);
    }
    return instructions;
}