#include "ast/expression.hpp"
#include "ast/statement.hpp"

#include <iostream>
#include <map>

using namespace toyc::ast;

llvm::Value *NBinaryOperator::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NBlock *parent) {
    llvm::Value *lhsValue = lhs->codegen(context, module, builder, parent);
    llvm::Value *rhsValue = rhs->codegen(context, module, builder, parent);
    llvm::Value *result = nullptr;

    if (nullptr == lhsValue || nullptr == rhsValue) {
        return nullptr;
    }

    std::map<BineryOperator, std::function<llvm::Value *(llvm::IRBuilder<> &, llvm::Value *, llvm::Value *)>> binaryOps = {
        {AND, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateAnd(lhs, rhs, "and"); }},
        {OR, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateOr(lhs, rhs, "or"); }},
        {ADD, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateAdd(lhs, rhs, "add"); }},
        {SUB, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateSub(lhs, rhs, "sub"); }},
        {MUL, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateMul(lhs, rhs, "mul"); }},
        {DIV, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateFDiv(lhs, rhs, "div"); }},
        {MOD, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateFRem(lhs, rhs, "mod"); }},
        {LEFT, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateShl(lhs, rhs, "left"); }},
        {RIGHT, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateLShr(lhs, rhs, "right"); }},
        {EQ, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateICmpEQ(lhs, rhs, "eq"); }},
        {NE, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateICmpNE(lhs, rhs, "ne"); }},
        {LE, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateICmpSLE(lhs, rhs, "le"); }},
        {GE, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateICmpSGE(lhs, rhs, "ge"); }},
        {L, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateICmpSLT(lhs, rhs, "lt"); }},
        {G, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateICmpSGT(lhs, rhs, "gt"); }},
        {BIT_AND, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateAnd(lhs, rhs, "bit_and"); }},
        {BIT_OR, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateOr(lhs, rhs, "bit_or"); }},
        {XOR, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateXor(lhs, rhs, "xor"); }}
    };

    if (0 != binaryOps.count(op)) {
        result = binaryOps[op](builder, lhsValue, rhsValue);
    } else {
        std::cerr << "Unknown binary operator: " << op << std::endl;
        return nullptr;
    }
    return result;
}

llvm::Value *NIdentifier::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NBlock *parent) {
    if (nullptr == parent) {
        std::cerr << "Error: Parent block is null" << std::endl;
        return nullptr;
    }

    llvm::AllocaInst *allocaInst = parent->findVariable(name);
    llvm::Value *value = nullptr;
    if (nullptr == allocaInst) {
        std::cerr << "Error: Variable not found: " << name << std::endl;
        return nullptr;
    }

    value = builder.CreateLoad(allocaInst->getAllocatedType(), allocaInst, name);
    if (nullptr == value) {
        std::cerr << "Error: Load failed for variable: " << name << std::endl;
    }

    return value;
}

llvm::Value *NInteger::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NBlock *parent) {
    return llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), value);
}

llvm::Value *NFloat::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NBlock *parent) {
    return llvm::ConstantFP::get(llvm::Type::getDoubleTy(context), value);
}

llvm::Value *NString::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NBlock *parent) {
    return llvm::ConstantDataArray::getString(context, value);
}
