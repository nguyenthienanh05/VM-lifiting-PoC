#pragma once

#include "disassembler.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <vector>

#define REGISTER_COUNT 8
#define REGISTER_SIZE 8

class Lifter {
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module;
    llvm::IRBuilder<> builder;
    std::vector<llvm::Value*> regs;

public:
    Lifter();
    void lift(const std::vector<Instruction>& instructions);
    void dump_output();
    void optimize();
    void saveOutput();
private:
    llvm::BasicBlock* entryBB;
    llvm::IntegerType* i8t;
    llvm::IntegerType* i32t;
    llvm::Function* mainFunc;
    llvm::FunctionCallee putCharFunc;
    llvm::FunctionCallee getCharFunc;
    llvm::Value* inputBuf;
    llvm::ArrayType* inputBufTy;
    size_t inputIndex = 0;
    void printChar(char value);
    void readChar();
    void mov_imm_op(uint8_t reg, uint8_t imm);
    void mov_op(uint8_t dst, uint8_t src);
    void and_op(uint8_t dst, uint8_t src1, uint8_t src2);
    void or_op(uint8_t dst, uint8_t src1, uint8_t src2);
    void xor_op(uint8_t dst, uint8_t src1, uint8_t src2);
    void pop_inp_op();

};
