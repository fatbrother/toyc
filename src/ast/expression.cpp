#include "ast/expression.hpp"
#include "ast/statement.hpp"

#include <iostream>
#include <map>

using namespace toyc::ast;

llvm::Value *NBinaryOperator::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) {
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
        {DIV, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateSDiv(lhs, rhs, "div"); }},
        {MOD, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateSRem(lhs, rhs, "mod"); }},
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

llvm::Value *NUnaryExpression::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) {
    llvm::Value *value = nullptr;
    llvm::Value *one = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 1);

    if (nullptr == expr) {
        std::cerr << "Error: Expression is null" << std::endl;
        return nullptr;
    }

    if (NOT == op) {
        value = expr->codegen(context, module, builder, parent);
        value = builder.CreateNot(value, "not");
    } else {
        llvm::Value *tmp = nullptr;
        switch (op) {
            case L_INC:
                value = expr->codegen(context, module, builder, parent);
                value = builder.CreateAdd(value, one, "inc");
                builder.CreateStore(value, expr->allocgen(context, module, builder, parent));
                break;
            case R_INC:
                value = expr->codegen(context, module, builder, parent);
                tmp = builder.CreateAdd(value, one, "inc");
                builder.CreateStore(tmp, expr->allocgen(context, module, builder, parent));
                break;
            case L_DEC:
                value = expr->codegen(context, module, builder, parent);
                value = builder.CreateSub(value, one, "dec");
                builder.CreateStore(value, expr->allocgen(context, module, builder, parent));
                break;
            case R_DEC:
                value = expr->codegen(context, module, builder, parent);
                tmp = builder.CreateSub(value, one, "dec");
                builder.CreateStore(tmp, expr->allocgen(context, module, builder, parent));
                break;
            default:
                std::cerr << "Unknown unary operator: " << op << std::endl;
                return nullptr;
        }
    }

    return value;
}

llvm::Value *NIdentifier::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) {
    if (nullptr == parent) {
        std::cerr << "Error: Parent block is null" << std::endl;
        return nullptr;
    }

    llvm::AllocaInst *allocaInst = parent->lookupVariable(name);
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

llvm::AllocaInst *NIdentifier::allocgen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) {
    if (nullptr == parent) {
        std::cerr << "Error: Parent block is null" << std::endl;
        return nullptr;
    }

    llvm::AllocaInst *allocaInst = parent->lookupVariable(name);
    if (nullptr == allocaInst) {
        std::cerr << "Error: Variable not found: " << name << std::endl;
        return nullptr;
    }

    return allocaInst;
}

llvm::Value *NAssignment::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) {
    llvm::AllocaInst *lhsAlloca = lhs->allocgen(context, module, builder, parent);
    llvm::Value *rhsValue = rhs->codegen(context, module, builder, parent);
    if (nullptr == rhsValue || nullptr == lhsAlloca) {
        std::cerr << "Error: Assignment failed due to null values" << std::endl;
        return nullptr;
    }

    if (lhsAlloca->getAllocatedType() != rhsValue->getType()) {
        std::cerr << "Error: Type mismatch in assignment" << std::endl;
        return nullptr;
    }
    builder.CreateStore(rhsValue, lhsAlloca);
    return rhsValue;
}

llvm::Value *NArguments::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) {
    if (nullptr == expr) {
        std::cerr << "Error: Expression is null" << std::endl;
        return nullptr;
    }
    return expr->codegen(context, module, builder, parent);
}

llvm::Value *NFunctionCall::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) {
    std::vector<llvm::Value *> args;
    llvm::Function *function = module.getFunction(name);
    llvm::Value *res = nullptr;
    if (nullptr == function) {
        std::cerr << "Error: Function not found: " << name << std::endl;
        return nullptr;
    }

    for (NArguments *argNode = argNodes; argNode != nullptr; argNode = argNode->next) {
        llvm::Value *argValue = argNode->codegen(context, module, builder, parent);
        if (nullptr == argValue) {
            std::cerr << "Error: Argument value is null" << std::endl;
            return nullptr;
        }
        args.push_back(argValue);
    }

    if (function->getReturnType()->isVoidTy()) {
        res = builder.CreateCall(function, args);
    } else {
        res = builder.CreateCall(function, args, "calltmp");
    }

    return res;
}

llvm::Value *NInteger::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), value);
}

llvm::Value *NFloat::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) {
    return llvm::ConstantFP::get(llvm::Type::getDoubleTy(context), value);
}

llvm::Value *NString::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) {
    return llvm::ConstantDataArray::getString(context, value);
}
