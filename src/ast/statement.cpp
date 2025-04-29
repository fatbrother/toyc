#include "ast/statement.hpp"

#include <iostream>

using namespace toyc::ast;

void NDeclarationStatement::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) {
    llvm::Type *llvmType = type->getLLVMType(context);

    if (nullptr == llvmType) {
        std::cerr << "Error: Type is null" << std::endl;
        return;
    }

    for (auto currentDeclarator = declarator; currentDeclarator != nullptr; currentDeclarator = currentDeclarator->next) {
        llvm::Value *value = currentDeclarator->codegen(context, module, builder, parent);
        llvm::AllocaInst *allocaInst = builder.CreateAlloca(llvmType, nullptr, currentDeclarator->getName());

        if (nullptr == allocaInst) {
            std::cerr << "Error: AllocaInst is null" << std::endl;
            return;
        }

        if (nullptr != value) {
            builder.CreateStore(value, allocaInst);
        }

        if (nullptr != parent) {
            parent->variables[currentDeclarator->getName()] = allocaInst;
        }
    }
}

void NBlock::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) {
    llvm::BasicBlock *block = llvm::BasicBlock::Create(context, name, parentFunction);
    builder.SetInsertPoint(block);

    if (nullptr != parentFunction) {
        for (auto &arg : parentFunction->args()) {
            llvm::AllocaInst *allocaInst = builder.CreateAlloca(arg.getType(), nullptr, arg.getName());
            builder.CreateStore(&arg, allocaInst);
            variables[arg.getName().str()] = allocaInst;
        }
    }

    for (NStatement *stmt = statements; stmt != nullptr; stmt = stmt->next) {
        stmt->setParent(this);
        stmt->codegen(context, module, builder);
    }
}

llvm::AllocaInst *NBlock::findVariable(const std::string &name) {
    auto it = variables.find(name);
    if (it != variables.end()) {
        return it->second;
    }
    if (nullptr != parent) {
        return parent->findVariable(name);
    }
    return nullptr;
}

void NReturnStatement::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) {
    if (nullptr != expression) {
        llvm::Value *value = expression->codegen(context, module, builder, parent);
        if (nullptr == value) {
            std::cerr << "Error: Return value is null" << std::endl;
            return;
        }
        builder.CreateRet(value);
    } else {
        builder.CreateRetVoid();
    }
}
