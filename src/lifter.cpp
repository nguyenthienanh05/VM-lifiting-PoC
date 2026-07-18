#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/raw_ostream.h>

#include <unordered_map>

#include "lifter.h"

Lifter::Lifter() : module(std::make_unique<llvm::Module>("vm_module", context)), builder(context) {
    i8t = builder.getInt8Ty();
    i32t = builder.getInt32Ty();
    
    llvm::FunctionType* funcType = llvm::FunctionType::get(i32t, {}, false);

    mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module.get());
    getCharFunc = module->getOrInsertFunction("getchar", llvm::FunctionType::get(i32t, {} ,false));
    putCharFunc = module->getOrInsertFunction("putchar", llvm::FunctionType::get(i32t, {i32t}, false));

    entryBB = llvm::BasicBlock::Create(context, "entry", mainFunc);
    builder.SetInsertPoint(entryBB);


    inputBufTy = llvm::ArrayType::get(i8t, 68);
    inputBuf = builder.CreateAlloca(inputBufTy, nullptr, "inputBuf");


    regs = std::vector<llvm::Value*>(REGISTER_COUNT, nullptr);
    for (int i = 0; i < REGISTER_COUNT; ++i) {
        regs[i] = builder.CreateAlloca(i8t, nullptr, "r" + std::to_string(i));
        builder.CreateStore(builder.getInt8(0), regs[i]);
    }

}

void Lifter::lift(const std::vector<Instruction>& instructions) {
    for (size_t i = 0; i < instructions.size(); i++) {
        const auto& inst = instructions[i];

        // std::cout << inst.name << std::endl;
        if (inst.name == "PRINT")
            printChar(inst.operands[0]);
        else if (inst.name == "GETCHAR")
            readChar();
        else if (inst.name == "MOV_IMM")
            mov_imm_op(inst.operands[0], inst.operands[1]);
        else if (inst.name == "MOV")
            mov_op(inst.operands[0], inst.operands[1]);
        else if (inst.name == "AND")
            and_op(inst.operands[0], inst.operands[1], inst.operands[2]);
        else if (inst.name == "OR")
            or_op(inst.operands[0], inst.operands[1], inst.operands[2]);
        else if (inst.name == "XOR")
            xor_op(inst.operands[0], inst.operands[1], inst.operands[2]);
        else if (inst.name == "POP_INP")
            pop_inp_op();
    }

    if (!builder.GetInsertBlock()->getTerminator()) {
          llvm::Value* r0 = builder.CreateLoad(i8t, regs[0]);
            llvm::Value* ret = builder.CreateZExt(r0, i32t);
            builder.CreateRet(ret);
    }
}

void Lifter::printChar(char value) {
    llvm::Value* charValue = builder.getInt32(static_cast<uint8_t>(value));
    builder.CreateCall(putCharFunc, {charValue});
}

void Lifter::readChar() {
    llvm::Value* charValue = builder.CreateCall(getCharFunc, {});
    llvm::Value* truncatedValue = builder.CreateTrunc(charValue, i8t);

    llvm::Value* inputPtr = builder.CreateGEP(inputBufTy, inputBuf, {builder.getInt32(0), builder.getInt32(inputIndex)});

    builder.CreateStore(truncatedValue, inputPtr);
    inputIndex++;
}

void Lifter::mov_imm_op(uint8_t reg, uint8_t imm) {
    if (reg >= REGISTER_COUNT) {
        throw std::runtime_error("Invalid register index: " + std::to_string(reg));
    }

    llvm::Value* regPtr = regs[reg];
    llvm::Value* immValueLLVM = builder.getInt8(imm);
    builder.CreateStore(immValueLLVM, regPtr);
}


void Lifter::mov_op(uint8_t dst, uint8_t src) {
    if (dst >= REGISTER_COUNT || src >= REGISTER_COUNT) {
        throw std::runtime_error("Invalid register index: dst=" + std::to_string(dst) + ", src=" + std::to_string(src));
    }

    llvm::Value* srcPtr = regs[src];
    llvm::Value* dstPtr = regs[dst];

    llvm::Value* valueToMove = builder.CreateLoad(i8t, srcPtr);
    builder.CreateStore(valueToMove, dstPtr);
}

void Lifter::and_op(uint8_t dst, uint8_t src1, uint8_t src2) {
    if (dst >= REGISTER_COUNT || src1 >= REGISTER_COUNT || src2 >= REGISTER_COUNT) {
        throw std::runtime_error("Invalid register index: dst=" + std::to_string(dst) + ", src1=" + std::to_string(src1) + ", src2=" + std::to_string(src2));
    }

    llvm::Value* src1Ptr = regs[src1];
    llvm::Value* src2Ptr = regs[src2];
    llvm::Value* dstPtr = regs[dst];

    llvm::Value* value1 = builder.CreateLoad(i8t, src1Ptr);
    llvm::Value* value2 = builder.CreateLoad(i8t, src2Ptr);
    llvm::Value* andResult = builder.CreateAnd(value1, value2);
    builder.CreateStore(andResult, dstPtr);
}

void Lifter::or_op(uint8_t dst, uint8_t src1, uint8_t src2) {
    if (dst >= REGISTER_COUNT || src1 >= REGISTER_COUNT || src2 >= REGISTER_COUNT) {
        throw std::runtime_error("Invalid register index: dst=" + std::to_string(dst) + ", src1=" + std::to_string(src1) + ", src2=" + std::to_string(src2));
    }

    llvm::Value* src1Ptr = regs[src1];
    llvm::Value* src2Ptr = regs[src2];
    llvm::Value* dstPtr = regs[dst];

    llvm::Value* value1 = builder.CreateLoad(i8t, src1Ptr);
    llvm::Value* value2 = builder.CreateLoad(i8t, src2Ptr);
    llvm::Value* orResult = builder.CreateOr(value1, value2);
    builder.CreateStore(orResult, dstPtr);
}

void Lifter::xor_op(uint8_t dst, uint8_t src1, uint8_t src2) {
    if (dst >= REGISTER_COUNT || src1 >= REGISTER_COUNT || src2 >= REGISTER_COUNT) {
        throw std::runtime_error("Invalid register index: dst=" + std::to_string(dst) + ", src1=" + std::to_string(src1) + ", src2=" + std::to_string(src2));
    }

    llvm::Value* src1Ptr = regs[src1];
    llvm::Value* src2Ptr = regs[src2];
    llvm::Value* dstPtr = regs[dst];

    llvm::Value* value1 = builder.CreateLoad(i8t, src1Ptr);
    llvm::Value* value2 = builder.CreateLoad(i8t, src2Ptr);
    llvm::Value* xorResult = builder.CreateXor(value1, value2);
    builder.CreateStore(xorResult, dstPtr);
}

void Lifter::pop_inp_op() {
    inputIndex--;
    llvm::Value* inputPtr = builder.CreateGEP(inputBufTy, inputBuf, {builder.getInt32(0), builder.getInt32(inputIndex)});
    llvm::Value* valueToPop = builder.CreateLoad(i8t, inputPtr);
    llvm::Value* dstPtr = regs[0];
    builder.CreateStore(valueToPop, dstPtr);
}

void Lifter::dump_output () {
    module->print(llvm::outs(), nullptr);
}

void Lifter::optimize() {
    if (llvm::verifyModule(*module, &llvm::errs())) {
        throw std::runtime_error("Generated invalid LLVM IR before optimization");
    }

    llvm::PassBuilder PB;
    llvm::LoopAnalysisManager LAM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager CGAM;
    llvm::ModuleAnalysisManager MAM;

    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O1);
    MPM.run(*module, MAM);

    if (llvm::verifyModule(*module, &llvm::errs())) {
        throw std::runtime_error("Optimizer produced invalid LLVM IR");
    }
}

void Lifter::saveOutput()
{
    std::error_code      EC;
    llvm::raw_fd_ostream outputFile("devirt.ll", EC);

    if (EC)
        llvm::errs() << "[!] Error writing to file: " << EC.message() << "\n";
    else
        module->print(outputFile, nullptr);
}
