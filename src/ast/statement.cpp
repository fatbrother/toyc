#include "ast/statement.hpp"

#include <iostream>

using namespace toyc::ast;

NStatement::~NStatement() {
    SAFE_DELETE(parent);
    SAFE_DELETE(next);
}

llvm::Value *NExpressionStatement::codegen(ASTContext &context) {
    if (nullptr != expression) {
        expression->codegen(context);
    }

    return nullptr;
}

llvm::Value *NDeclarationStatement::codegen(ASTContext &context) {
    llvm::Type *llvmType = type->getLLVMType(context.llvmContext);

    if (nullptr == llvmType) {
        std::cerr << "Error: Type is null" << std::endl;
        return nullptr;
    }

    for (auto currentDeclarator = declarator; currentDeclarator != nullptr; currentDeclarator = currentDeclarator->next) {
        if (context.variableTable->lookupVariable(currentDeclarator->getName(), false).first != nullptr) {
            std::cerr << "Error: Variable already declared in this scope: " << currentDeclarator->getName() << std::endl;
            return nullptr;
        }

        llvm::AllocaInst *allocaInst = context.builder.CreateAlloca(llvmType, nullptr, currentDeclarator->getName());
        context.variableTable->insertVariable(currentDeclarator->getName(), allocaInst, type);

        auto [value, valType] = currentDeclarator->codegen(context);

        if (nullptr == allocaInst) {
            std::cerr << "Error: AllocaInst is null" << std::endl;
            return nullptr;
        }

        if (nullptr != value) {
            value = typeCast(value, valType, type, context.llvmContext, context.builder);
            context.builder.CreateStore(value, allocaInst);
        }

    }

    return nullptr;
}

llvm::Value *NBlock::codegen(ASTContext &context) {
    llvm::Function *parentFunction = context.currentFunction->getFunction();
    llvm::BasicBlock *block = llvm::BasicBlock::Create(context.llvmContext, name, parentFunction);

    context.builder.SetInsertPoint(block);
    context.pushScope();

    if (true == context.isInitializingFunction) {
        auto *param = context.currentFunction->getParams();
        for (auto &arg : context.currentFunction->getFunction()->args()) {
            llvm::AllocaInst *allocaInst = context.builder.CreateAlloca(arg.getType(), nullptr, arg.getName());
            context.builder.CreateStore(&arg, allocaInst);
            context.variableTable->insertVariable(arg.getName().str(), allocaInst, param->getVarType());
            param = param->next;
        }
    }
    context.isInitializingFunction = false;

    for (NStatement *stmt = statements; stmt != nullptr; stmt = stmt->next) {
        if (nullptr != parent) {
            stmt->setParent(parent);
        }
        stmt->codegen(context);
    }

    context.popScope();

    if (nullptr != nextBlock) {
        context.builder.CreateBr(nextBlock);
        context.builder.SetInsertPoint(nextBlock);
    }

    return block;
}

llvm::Value *NReturnStatement::codegen(ASTContext &context) {
    if (nullptr != expression) {
        auto [value, type] = expression->codegen(context);
        value = typeCast(
            value,
            type,
            context.currentFunction->getReturnType(),
            context.llvmContext,
            context.builder);
        if (nullptr == value) {
            std::cerr << "Error: Return value is null" << std::endl;
            return nullptr;
        }
        context.builder.CreateRet(value);
    } else {
        context.builder.CreateRetVoid();
    }

    return nullptr;
}

llvm::Value *NIfStatement::codegen(ASTContext &context) {
    llvm::Function *function = context.currentFunction->getFunction();
    llvm::BasicBlock *thenBlock = nullptr;
    llvm::BasicBlock *elseBlock = nullptr;
    llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(context.llvmContext, "if_merge", function);
    llvm::BasicBlock *conditionBlock = llvm::BasicBlock::Create(context.llvmContext, "if_condition", function);
    llvm::Value *conditionValue = nullptr;

    context.builder.CreateBr(conditionBlock);
    context.builder.SetInsertPoint(conditionBlock);
    conditionValue = conditionNode->codegen(context).first;
    if (nullptr == conditionValue) {
        std::cerr << "Error: Condition value is null" << std::endl;
        return nullptr;
    }

    thenBlockNode->setParent(parent);
    thenBlockNode->setName("if_then");
    thenBlockNode->setNextBlock(mergeBlock);
    thenBlock = static_cast<llvm::BasicBlock *>(thenBlockNode->codegen(context));
    if (nullptr == thenBlock) {
        std::cerr << "Error: Then block is null" << std::endl;
        return nullptr;
    }

    if (nullptr != elseBlockNode) {
        elseBlockNode->setParent(parent);
        elseBlockNode->setName("if_else");
        elseBlockNode->setNextBlock(mergeBlock);
        elseBlock = static_cast<llvm::BasicBlock *>(elseBlockNode->codegen(context));
        if (nullptr == elseBlock) {
            std::cerr << "Error: Else block is null" << std::endl;
            return nullptr;
        }
    }

    context.builder.SetInsertPoint(conditionBlock);
    context.builder.CreateCondBr(conditionValue, thenBlock, (nullptr != elseBlock) ? elseBlock : mergeBlock);
    context.builder.SetInsertPoint(mergeBlock);
    return mergeBlock;
}

llvm::Value *NForStatement::codegen(ASTContext &context) {
    llvm::Function *function = context.currentFunction->getFunction();
    llvm::BasicBlock *afterBlock = llvm::BasicBlock::Create(context.llvmContext, "for_after", function);
    llvm::BasicBlock *loopCondition = llvm::BasicBlock::Create(context.llvmContext, "for_condition", function);
    llvm::BasicBlock *loopIncrement = llvm::BasicBlock::Create(context.llvmContext, "for_increment", function);
    llvm::BasicBlock *loopBody = nullptr;

    initializationNode->setParent(this);
    initializationNode->codegen(context);
    context.builder.CreateBr(loopCondition);

    bodyNode->setParent(this);
    bodyNode->setName("for_body");
    bodyNode->setNextBlock(loopIncrement);
    loopBody = static_cast<llvm::BasicBlock *>(bodyNode->codegen(context));
    if (nullptr == loopBody) {
        std::cerr << "Error: Loop body is null" << std::endl;
        return nullptr;
    }

    incrementNode->codegen(context);
    context.builder.CreateBr(loopCondition);
    context.builder.SetInsertPoint(loopCondition);
    auto [conditionValue, conditionType] = conditionNode->codegen(context);
    conditionValue = typeCast(conditionValue, conditionType, VAR_TYPE_BOOL, context.llvmContext, context.builder);
    if (nullptr == conditionValue) {
        context.builder.CreateBr(loopBody);
    } else {
        context.builder.CreateCondBr(conditionValue, loopBody, afterBlock);
    }

    context.builder.SetInsertPoint(afterBlock);

    return afterBlock;
}

llvm::Value *NWhileStatement::codegen(ASTContext &context) {
    llvm::Function *function = context.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *loopCondition = llvm::BasicBlock::Create(context.llvmContext, "while_condition", function);
    llvm::BasicBlock *loopBody = nullptr;
    llvm::BasicBlock *afterBlock = llvm::BasicBlock::Create(context.llvmContext, "while_after", function);
    llvm::BasicBlock *previousBlock = context.builder.GetInsertBlock();

    context.builder.SetInsertPoint(loopCondition);
    auto [conditionValue, conditionType] = conditionNode->codegen(context);
    conditionValue = typeCast(conditionValue,
                              conditionType,
                              VAR_TYPE_BOOL,
                              context.llvmContext,
                              context.builder);
    if (nullptr == conditionValue) {
        std::cerr << "Error: Condition value is null" << std::endl;
        return nullptr;
    }

    bodyNode->setParent(this);
    bodyNode->setName("while_body");
    bodyNode->setNextBlock(loopCondition);
    loopBody = static_cast<llvm::BasicBlock *>(bodyNode->codegen(context));
    if (nullptr == loopBody) {
        std::cerr << "Error: Loop body is null" << std::endl;
        return nullptr;
    }

    context.builder.SetInsertPoint(loopCondition);
    context.builder.CreateCondBr(conditionValue, loopBody, afterBlock);

    context.builder.SetInsertPoint(previousBlock);
    if (true == isDoWhile) {
        context.builder.CreateBr(loopBody);
    } else {
        context.builder.CreateBr(loopCondition);
    }

    context.builder.SetInsertPoint(afterBlock);

    return afterBlock;
}
