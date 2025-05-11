#include "ast/statement.hpp"

#include <iostream>

using namespace toyc::ast;

NStatement::~NStatement() {
    SAFE_DELETE(parent);
    SAFE_DELETE(next);
}

llvm::AllocaInst *NParentStatement::lookupVariable(const std::string &name) {
    auto it = variables.find(name);
    if (it != variables.end()) {
        return it->second;
    }

    return (nullptr != parent) ? parent->lookupVariable(name) : nullptr;
}

void NParentStatement::insertVariable(const std::string &name, llvm::AllocaInst *allocaInst) {
    variables[name] = allocaInst;
}

llvm::Value *NDeclarationStatement::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) {
    llvm::Type *llvmType = type->getLLVMType(context);

    if (nullptr == llvmType) {
        std::cerr << "Error: Type is null" << std::endl;
        return nullptr;
    }

    for (auto currentDeclarator = declarator; currentDeclarator != nullptr; currentDeclarator = currentDeclarator->next) {
        llvm::Value *value = currentDeclarator->codegen(context, module, builder, parent);
        llvm::AllocaInst *allocaInst = builder.CreateAlloca(llvmType, nullptr, currentDeclarator->getName());

        if (nullptr == allocaInst) {
            std::cerr << "Error: AllocaInst is null" << std::endl;
            return nullptr;
        }

        if (nullptr != value) {
            builder.CreateStore(value, allocaInst);
        }

        if (nullptr != parent) {
            parent->insertVariable(currentDeclarator->getName(), allocaInst);
        }
    }

    return nullptr;
}

llvm::Value *NBlock::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) {
    llvm::BasicBlock *block = llvm::BasicBlock::Create(context, name, parentFunction);

    builder.SetInsertPoint(block);

    if (true == isFunctionBlock) {
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

    if (nullptr != nextBlock) {
        builder.CreateBr(nextBlock);
        builder.SetInsertPoint(nextBlock);
    }

    return block;
}

llvm::Value *NReturnStatement::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) {
    if (nullptr != expression) {
        llvm::Value *value = expression->codegen(context, module, builder, parent);
        if (nullptr == value) {
            std::cerr << "Error: Return value is null" << std::endl;
            return nullptr;
        }
        builder.CreateRet(value);
    } else {
        builder.CreateRetVoid();
    }

    return nullptr;
}

llvm::Value *NIfStatement::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) {
    llvm::Function *function = builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *thenBlock = nullptr;
    llvm::BasicBlock *elseBlock = nullptr;
    llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(context, "if_merge", function);
    llvm::BasicBlock *conditionBlock = llvm::BasicBlock::Create(context, "if_condition", function);
    llvm::Value *conditionValue = nullptr;

    builder.CreateBr(conditionBlock);
    builder.SetInsertPoint(conditionBlock);
    conditionValue = conditionNode->codegen(context, module, builder, this);
    if (nullptr == conditionValue) {
        std::cerr << "Error: Condition value is null" << std::endl;
        return nullptr;
    }

    thenBlockNode->setParent(parent);
    thenBlockNode->setName("if_then");
    thenBlockNode->setNextBlock(mergeBlock);
    thenBlockNode->setParentFunction(function);
    thenBlock = static_cast<llvm::BasicBlock *>(thenBlockNode->codegen(context, module, builder));
    if (nullptr == thenBlock) {
        std::cerr << "Error: Then block is null" << std::endl;
        return nullptr;
    }

    if (nullptr != elseBlockNode) {
        elseBlockNode->setParent(parent);
        elseBlockNode->setName("if_else");
        elseBlockNode->setNextBlock(mergeBlock);
        elseBlockNode->setParentFunction(function);
        elseBlock = static_cast<llvm::BasicBlock *>(elseBlockNode->codegen(context, module, builder));
        if (nullptr == elseBlock) {
            std::cerr << "Error: Else block is null" << std::endl;
            return nullptr;
        }
    }

    builder.SetInsertPoint(conditionBlock);
    builder.CreateCondBr(conditionValue, thenBlock, (nullptr != elseBlock) ? elseBlock : mergeBlock);
    builder.SetInsertPoint(mergeBlock);
    return mergeBlock;
}

llvm::Value *NForStatement::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) {
    llvm::Function *function = builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *afterBlock = llvm::BasicBlock::Create(context, "for_after", function);
    llvm::BasicBlock *loopCondition = llvm::BasicBlock::Create(context, "for_condition", function);
    llvm::BasicBlock *loopIncrement = llvm::BasicBlock::Create(context, "for_increment", function);
    llvm::BasicBlock *loopBody = nullptr;

    initializationNode->setParent(this);
    llvm::Value *initValue = initializationNode->codegen(context, module, builder);
    builder.CreateBr(loopCondition);

    bodyNode->setParent(this);
    bodyNode->setName("for_body");
    bodyNode->setNextBlock(loopIncrement);
    bodyNode->setParentFunction(function);
    loopBody = static_cast<llvm::BasicBlock *>(bodyNode->codegen(context, module, builder));
    if (nullptr == loopBody) {
        std::cerr << "Error: Loop body is null" << std::endl;
        return nullptr;
    }

    llvm::Value *incrementValue = incrementNode->codegen(context, module, builder, this);
    builder.CreateBr(loopCondition);
    builder.SetInsertPoint(loopCondition);
    llvm::Value *conditionValue = conditionNode->codegen(context, module, builder, this);
    if (nullptr == conditionValue) {
        builder.CreateBr(loopBody);
    } else {
        builder.CreateCondBr(conditionValue, loopBody, afterBlock);
    }

    builder.SetInsertPoint(afterBlock);

    return afterBlock;
}
