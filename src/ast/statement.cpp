#include "ast/statement.hpp"

#include "ast/external_definition.hpp"
#include "ast/structure.hpp"
#include "utility/raii_guard.hpp"

#include <iostream>

using namespace toyc::ast;

NStatement::~NStatement() {
    SAFE_DELETE(parent);
    SAFE_DELETE(next);
}

CodegenResult NExpressionStatement::codegen(ASTContext &context) {
    if (nullptr == expression) {
        return CodegenResult();
    }

    return expression->codegen(context);
}

CodegenResult NDeclarationStatement::codegen(ASTContext &context) {
    llvm::Type *llvmType = nullptr;
    llvm::AllocaInst *allocaInst = nullptr;

    if ("StructType" == type->getType()) {
        auto structType = static_cast<NStructType *>(type.get());
        if (nullptr != structType->members) {
            structType->codegen(context);
            context.typeTable->insert(structType->name, type);
        }
    }

    CodegenResult typeResult = type->getLLVMType(context);
    if (false == typeResult.isSuccess() || nullptr == typeResult.getLLVMType()) {
        return typeResult << CodegenResult("Failed to get LLVM type for declaration");
    }
    llvmType = typeResult.getLLVMType();

    for (auto currentDeclarator = declarator; currentDeclarator != nullptr; currentDeclarator = currentDeclarator->next) {
        if (true == context.variableTable->lookup(currentDeclarator->getName(), false).first) {
            return CodegenResult("Variable already declared in this scope: " + currentDeclarator->getName());
        }

        allocaInst = context.builder.CreateAlloca(llvmType, nullptr, currentDeclarator->getName());

        // copy type and pointer level from declaration
        NTypePtr newType = nullptr;
        if (declarator->pointerLevel > 0) {
            newType = std::make_shared<NType>(VarType::VAR_TYPE_PTR, type, currentDeclarator->pointerLevel);
        } else {
            newType = type;
        }
        context.variableTable->insert(currentDeclarator->getName(), std::make_pair(allocaInst, newType));

        if (true == currentDeclarator->isNonInitialized()) {
            continue;
        }

        CodegenResult codegenResult = currentDeclarator->codegen(context);
        llvm::Value *value = codegenResult.getValue();
        NTypePtr valType = codegenResult.getType();
        if ((false == codegenResult.isSuccess()) || (nullptr == value)) {
            return codegenResult << CodegenResult("Initializer codegen failed for variable: " + currentDeclarator->getName());
        } else {
            CodegenResult castResult = typeCast(value, valType, type, context);
            if (false == castResult.isSuccess() || nullptr == castResult.getValue()) {
                return castResult << CodegenResult("Type cast failed for initializer of variable: " + currentDeclarator->getName());
            }
            context.builder.CreateStore(value, allocaInst);
        }
    }

    return CodegenResult(allocaInst);
}

CodegenResult NBlock::codegen(ASTContext &context) {
    llvm::Function *parentFunction = context.currentFunction->getFunction();
    llvm::BasicBlock *block = llvm::BasicBlock::Create(context.llvmContext, name, parentFunction);

    context.builder.SetInsertPoint(block);

    auto scopeGuard = toyc::utility::makeScopeGuard([&context]() {
        context.popScope();
    });
    context.pushScope();

    if (true == context.isInitializingFunction) {
        auto *param = context.currentFunction->getParams();
        for (auto &arg : context.currentFunction->getFunction()->args()) {
            llvm::AllocaInst *allocaInst = context.builder.CreateAlloca(arg.getType(), nullptr, arg.getName());
            context.builder.CreateStore(&arg, allocaInst);
            context.variableTable->insert(arg.getName().str(), std::make_pair(allocaInst, param->getVarType()));
            param = param->next;
        }
    }
    context.isInitializingFunction = false;

    for (NStatement *stmt = statements; stmt != nullptr; stmt = stmt->next) {
        if (nullptr != parent) {
            stmt->setParent(parent);
        }

        CodegenResult stmtResult = stmt->codegen(context);
        if (false == stmtResult.isSuccess()) {
            return stmtResult << CodegenResult("Failed to generate code for block statement");
        }
    }


    if (nullptr != nextBlock && !context.builder.GetInsertBlock()->getTerminator()) {
        context.builder.CreateBr(nextBlock);
        context.builder.SetInsertPoint(nextBlock);
    }

    return CodegenResult(block);
}

CodegenResult NReturnStatement::codegen(ASTContext &context) {
    if (nullptr != expression) {
        CodegenResult exprResult = expression->codegen(context);
        llvm::Value *value = exprResult.getValue();
        NTypePtr type = exprResult.getType();
        if (false == exprResult.isSuccess() || nullptr == value) {
            return exprResult << CodegenResult("Failed to generate code for return expression");
        }
        CodegenResult castResult = typeCast(
            value,
            type,
            context.currentFunction->getReturnType(),
            context);
        if (false == castResult.isSuccess() || nullptr == castResult.getValue()) {
            return castResult << CodegenResult("Type cast failed for return statement");
        }
        return CodegenResult(context.builder.CreateRet(value));
    } else {
        return CodegenResult(context.builder.CreateRetVoid());
    }

    return CodegenResult();
}

CodegenResult NIfStatement::codegen(ASTContext &context) {
    llvm::Function *function = context.currentFunction->getFunction();
    llvm::BasicBlock *thenBlock = nullptr;
    llvm::BasicBlock *elseBlock = nullptr;
    llvm::BasicBlock *conditionBlock = llvm::BasicBlock::Create(context.llvmContext, "if_condition", function);
    llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(context.llvmContext, "if_merge", function);
    llvm::Value *conditionValue = nullptr;

    context.builder.CreateBr(conditionBlock);
    context.builder.SetInsertPoint(conditionBlock);
    CodegenResult condResult = conditionNode->codegen(context);
    conditionValue = condResult.getValue();
    if (false == condResult.isSuccess() || nullptr == conditionValue) {
        return condResult << CodegenResult("Condition code generation failed for if statement");
    }

    thenBlockNode->setParent(parent);
    thenBlockNode->setName("if_then");
    thenBlockNode->setNextBlock(mergeBlock);
    CodegenResult thenBlockResult = thenBlockNode->codegen(context);
    thenBlock = static_cast<llvm::BasicBlock *>(thenBlockResult.getValue());
    if (false == thenBlockResult.isSuccess() || nullptr == thenBlock) {
        return thenBlockResult << CodegenResult("Then block generation failed for if statement");
    }

    if (nullptr != elseBlockNode) {
        elseBlockNode->setParent(parent);
        elseBlockNode->setName("if_else");
        elseBlockNode->setNextBlock(mergeBlock);
        CodegenResult elseBlockResult = elseBlockNode->codegen(context);
        elseBlock = static_cast<llvm::BasicBlock *>(elseBlockResult.getValue());
        if (false == elseBlockResult.isSuccess() || nullptr == elseBlock) {
            return elseBlockResult << CodegenResult("Else block generation failed for if statement");
        }
    }

    context.builder.SetInsertPoint(conditionBlock);
    context.builder.CreateCondBr(conditionValue, thenBlock, (nullptr != elseBlock) ? elseBlock : mergeBlock);
    context.builder.SetInsertPoint(mergeBlock);
    return mergeBlock;
}

CodegenResult NForStatement::codegen(ASTContext &context) {
    llvm::Function *function = context.currentFunction->getFunction();
    llvm::BasicBlock *afterBlock = llvm::BasicBlock::Create(context.llvmContext, "for_after", function);
    llvm::BasicBlock *loopCondition = llvm::BasicBlock::Create(context.llvmContext, "for_condition", function);
    llvm::BasicBlock *loopIncrement = llvm::BasicBlock::Create(context.llvmContext, "for_increment", function);
    llvm::BasicBlock *loopBody = nullptr;

    initializationNode->setParent(this);
    initializationNode->codegen(context);
    context.builder.CreateBr(loopCondition);

    auto jumpGuard = toyc::utility::makeScopeGuard([&context]() {
        context.popJumpContext();
    });
    context.pushJumpContext(std::make_shared<NJumpContext>(loopIncrement, afterBlock));

    bodyNode->setParent(this);
    bodyNode->setName("for_body");
    bodyNode->setNextBlock(loopIncrement);
    CodegenResult bodyResult = bodyNode->codegen(context);
    loopBody = static_cast<llvm::BasicBlock *>(bodyResult.getValue());
    if (false == bodyResult.isSuccess() || nullptr == loopBody) {
        return bodyResult << CodegenResult("For loop body generation failed");
    }

    incrementNode->codegen(context);
    context.builder.CreateBr(loopCondition);
    context.builder.SetInsertPoint(loopCondition);
    CodegenResult condResult = conditionNode->codegen(context);
    llvm::Value *conditionValue = condResult.getValue();
    NTypePtr conditionType = condResult.getType();
    if (false == condResult.isSuccess() || nullptr == conditionValue) {
        return condResult << CodegenResult("For loop condition generation failed");
    }
    CodegenResult conditionCastResult = typeCast(
        conditionValue,
        conditionType,
        VAR_TYPE_BOOL,
        context);
    if (false == conditionCastResult.isSuccess()) {
        return conditionCastResult << CodegenResult("Type cast failed for for loop condition");
    }
    if (nullptr == conditionValue) {
        context.builder.CreateBr(loopBody);
    } else {
        context.builder.CreateCondBr(conditionValue, loopBody, afterBlock);
    }

    context.builder.SetInsertPoint(afterBlock);

    return CodegenResult(afterBlock);
}

CodegenResult NWhileStatement::codegen(ASTContext &context) {
    llvm::Function *function = context.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *loopCondition = llvm::BasicBlock::Create(context.llvmContext, "while_condition", function);
    llvm::BasicBlock *loopBody = nullptr;
    llvm::BasicBlock *afterBlock = llvm::BasicBlock::Create(context.llvmContext, "while_after", function);
    llvm::BasicBlock *previousBlock = context.builder.GetInsertBlock();

    context.builder.SetInsertPoint(loopCondition);
    CodegenResult condResult = conditionNode->codegen(context);
    llvm::Value *conditionValue = condResult.getValue();
    NTypePtr conditionType = condResult.getType();
    if (false == condResult.isSuccess() || nullptr == conditionValue) {
        return condResult << CodegenResult("While loop condition generation failed");
    }
    CodegenResult castResult = typeCast(
        conditionValue,
        conditionType,
        VAR_TYPE_BOOL,
        context);
    if (false == castResult.isSuccess() || nullptr == castResult.getValue()) {
        return castResult << CodegenResult("Type cast failed for while loop condition");
    }
    conditionValue = castResult.getValue();

    auto jumpGuard = toyc::utility::makeScopeGuard([&context]() {
        context.popJumpContext();
    });
    context.pushJumpContext(std::make_shared<NJumpContext>(loopCondition, afterBlock));

    bodyNode->setParent(this);
    bodyNode->setName("while_body");
    bodyNode->setNextBlock(loopCondition);
    CodegenResult bodyResult = bodyNode->codegen(context);
    loopBody = static_cast<llvm::BasicBlock *>(bodyResult.getValue());
    if (false == bodyResult.isSuccess() || nullptr == loopBody) {
        return bodyResult << CodegenResult("While loop body generation failed");
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

CodegenResult NBreakStatement::codegen(ASTContext &context) {
    auto jumpCtx = context.getCurrentJumpContext();
    if (!jumpCtx) {
        return CodegenResult("Break statement not within a loop or switch");
    }

    if (!jumpCtx->supportsBreak()) {
        return CodegenResult("Break statement not supported in current context");
    }

    // Get the break target from the jump context
    llvm::BasicBlock *breakTarget = jumpCtx->getBreakTarget();
    context.builder.CreateBr(breakTarget);

    // Create an unreachable block after the break to avoid dangling insert point
    llvm::Function *function = context.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *unreachableBlock = llvm::BasicBlock::Create(
        context.llvmContext, "after_break", function);
    context.builder.SetInsertPoint(unreachableBlock);

    return CodegenResult();
}

CodegenResult NContinueStatement::codegen(ASTContext &context) {
    auto jumpCtx = context.getCurrentJumpContext();
    if (!jumpCtx) {
        return CodegenResult("Continue statement not within a loop");
    }

    if (!jumpCtx->supportsContinue()) {
        return CodegenResult("Continue statement not supported in " +
                           jumpCtx->getContextName() + " context");
    }

    // Get the continue target from the jump context
    llvm::BasicBlock *continueTarget = jumpCtx->getContinueTarget();
    context.builder.CreateBr(continueTarget);

    // Create an unreachable block after the continue to avoid dangling insert point
    llvm::Function *function = context.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *unreachableBlock = llvm::BasicBlock::Create(
        context.llvmContext, "after_continue", function);
    context.builder.SetInsertPoint(unreachableBlock);

    return CodegenResult();
}

CodegenResult NLabelStatement::codegen(ASTContext &context) {
    llvm::Function *function = context.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *labelBlock = context.getLabel(label);

    if (labelBlock) {
        if (!labelBlock->empty()) {
            return CodegenResult("Label '" + label + "' is already defined");
        }
        context.pendingGotos.erase(label);
    } else {
        labelBlock = llvm::BasicBlock::Create(
            context.llvmContext, "label_" + label, function);

        context.registerLabel(label, labelBlock);
    }

    // Branch from current block to label block if no terminator
    if (!context.builder.GetInsertBlock()->getTerminator()) {
        context.builder.CreateBr(labelBlock);
    }

    // Set insert point to the label block
    context.builder.SetInsertPoint(labelBlock);

    // Generate code for the statement after the label
    if (statement) {
        CodegenResult result = statement->codegen(context);
        if (!result.isSuccess()) {
            return result << CodegenResult("Failed to generate code for statement after label");
        }
    }

    return CodegenResult(labelBlock);
}

CodegenResult NGotoStatement::codegen(ASTContext &context) {
    llvm::Function *function = context.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *targetBlock = context.getLabel(label);

    if (!targetBlock) {
        // Label not yet defined, create a placeholder block
        targetBlock = llvm::BasicBlock::Create(
            context.llvmContext, "label_" + label, function);

        llvm::BranchInst *branch = context.builder.CreateBr(targetBlock);
        context.registerLabel(label, targetBlock);
        context.pendingGotos.insert(label);
    } else {
        context.builder.CreateBr(targetBlock);
    }

    // Create an unreachable block after the goto
    llvm::BasicBlock *afterGoto = llvm::BasicBlock::Create(
        context.llvmContext, "after_goto", function);
    context.builder.SetInsertPoint(afterGoto);

    return CodegenResult();
}