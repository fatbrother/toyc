#include "ast/statement.hpp"

#include "ast/external_definition.hpp"
#include "utility/raii_guard.hpp"

#include <iostream>

using namespace toyc::ast;
using namespace toyc::utility;

NStatement::~NStatement() {
    SAFE_DELETE(parent);
    SAFE_DELETE(next);
}

StmtCodegenResult NExpressionStatement::codegen(ASTContext &context) {
    if (nullptr == expression) {
        return StmtCodegenResult();
    }

    ExprCodegenResult exprResult = expression->codegen(context);
    if (false == exprResult.isSuccess()) {
        return StmtCodegenResult("Failed to generate code for expression statement") << exprResult;
    }

    return StmtCodegenResult();
}

StmtCodegenResult NDeclarationStatement::codegen(ASTContext &context) {
    llvm::AllocaInst *allocaInst = nullptr;
    llvm::Type* type = context.typeManager->realize(typeIdx);
    if (nullptr == type) {
        return StmtCodegenResult("Failed to realize type from descriptor");
    }

    for (auto currentDeclarator = declarator; currentDeclarator != nullptr; currentDeclarator = currentDeclarator->next) {
        if (true == context.variableTable->lookup(currentDeclarator->getName(), false).first) {
            return StmtCodegenResult("Variable already declared in this scope: " + currentDeclarator->getName());
        }

        AllocCodegenResult allocResult;
        if (true == currentDeclarator->isArray()) {
            allocResult = createArrayAllocation(context, type, typeIdx, currentDeclarator);
        } else if (true == currentDeclarator->isPointer()) {
            allocResult = createPointerAllocation(context, type, typeIdx, currentDeclarator);
        } else {
            allocResult = createSingleAllocation(context, type, typeIdx, currentDeclarator);
        }

        if (false == allocResult.isSuccess()) {
            return StmtCodegenResult("Variable declaration codegen failed for variable: " + currentDeclarator->getName()) << allocResult;
        }

        allocaInst = static_cast<llvm::AllocaInst *>(allocResult.getAllocaInst());
        TypeIdx currTypeIdx = allocResult.getType();
        context.variableTable->insert(currentDeclarator->getName(), std::make_pair(allocaInst, currTypeIdx));

        if (true == currentDeclarator->isNonInitialized()) {
            continue;
        }

        llvm::Type* currType = context.typeManager->realize(currTypeIdx);
        if (nullptr != currType && true == currType->isArrayTy()) {
            NInitializerList* initList = dynamic_cast<NInitializerList*>(currentDeclarator->expr);
            StmtCodegenResult initResult = initializeArrayElements(allocaInst, currTypeIdx, initList, context);
            if (false == initResult.isSuccess()) {
                return initResult;
            }
            continue;
        }

        ExprCodegenResult codegenResult = currentDeclarator->codegen(context);
        llvm::Value *value = codegenResult.getValue();
        TypeIdx valTypeIdx = codegenResult.getType();
        if ((false == codegenResult.isSuccess()) || (nullptr == value)) {
            return StmtCodegenResult("Initializer codegen failed for variable: " + currentDeclarator->getName()) << codegenResult;
        } else {
            ExprCodegenResult castResult = context.typeManager->typeCast(value, valTypeIdx, currTypeIdx, context.builder);
            if (false == castResult.isSuccess() || nullptr == castResult.getValue()) {
                return StmtCodegenResult("Type cast failed for initializer of variable: " + currentDeclarator->getName()) << castResult;
            }
            context.builder.CreateStore(castResult.getValue(), allocaInst);
        }
    }

    return StmtCodegenResult();
}

AllocCodegenResult NDeclarationStatement::createArrayAllocation(ASTContext &context, llvm::Type *baseType, TypeIdx baseTypeIdx, NDeclarator* declarator) {
    if (false == declarator->isVLA) {
        std::vector<int> dimensions;
        for (auto sizeExpr : declarator->getArrayDimensions()) {
            dimensions.push_back(static_cast<NInteger*>(sizeExpr)->getValue());
        }
        TypeIdx arrayTypeIdx = context.typeManager->getArrayIdx(baseTypeIdx, dimensions);
        llvm::Type *arrayType = context.typeManager->realize(arrayTypeIdx);
        return createSingleAllocation(context, arrayType, arrayTypeIdx, declarator);
    }

    // Handle VLA allocation
    CodegenResult<llvm::Value *> sizeValue = declarator->getArraySizeValue(context);
    if (false == sizeValue.isSuccess()) {
        return AllocCodegenResult("Failed to generate size value for VLA") << sizeValue;
    }

    // CreateAlloca with array size returns a pointer to the array
    llvm::Value *vlaArrayPtr = context.builder.CreateAlloca(
        baseType,
        sizeValue.getData(),
        declarator->getName() + ".vla");

    // Wrap the VLA pointer in an alloca so it can be treated like a regular pointer
    llvm::Type *ptrType = vlaArrayPtr->getType();
    TypeIdx ptrTypeIdx = context.typeManager->getPointerIdx(baseTypeIdx, 1);
    llvm::AllocaInst *ptrStorage = context.builder.CreateAlloca(
        ptrType,
        nullptr,
        declarator->getName());

    context.builder.CreateStore(vlaArrayPtr, ptrStorage);

    return AllocCodegenResult(ptrStorage, ptrTypeIdx);
}

AllocCodegenResult NDeclarationStatement::createPointerAllocation(ASTContext &context, llvm::Type *baseType, TypeIdx baseTypeIdx, NDeclarator *declarator) {
    TypeIdx ptrTypeIdx = context.typeManager->getPointerIdx(baseTypeIdx, declarator->pointerLevel);
    llvm::Type *ptrType = context.typeManager->realize(ptrTypeIdx);
    return createSingleAllocation(context, ptrType, ptrTypeIdx, declarator);
}

AllocCodegenResult NDeclarationStatement::createSingleAllocation(ASTContext &context, llvm::Type *type, TypeIdx typeIdx, NDeclarator *declarator) {
    llvm::AllocaInst *allocaInst = context.builder.CreateAlloca(type, nullptr, declarator->getName());
    return AllocCodegenResult(allocaInst, typeIdx);
}

StmtCodegenResult NDeclarationStatement::initializeArrayElements(
    llvm::AllocaInst* allocaInst,
    TypeIdx arrayTypeIdx,
    NInitializerList* initList,
    ASTContext& context)
{
    if (nullptr == initList) {
        return StmtCodegenResult("Array must be initialized with initializer list");
    }

    llvm::Type* arrayType = context.typeManager->realize(arrayTypeIdx);
    auto* arrTc = dynamic_cast<const ArrayTypeCodegen*>(context.typeManager->get(arrayTypeIdx));
    TypeIdx elementTypeIdx = arrTc ? arrTc->getElementIdx() : InvalidTypeIdx;
    llvm::Type* elementType = context.typeManager->realize(elementTypeIdx);

    const std::vector<NExpression*>& elements = initList->getElements();

    for (size_t i = 0; i < elements.size(); i++) {
        std::vector<llvm::Value*> indices;
        indices.push_back(context.builder.getInt32(0));
        indices.push_back(context.builder.getInt32(i));

        llvm::Value* elementPtr = context.builder.CreateGEP(arrayType, allocaInst, indices);

        ExprCodegenResult elemResult = elements[i]->codegen(context);
        if (false == elemResult.isSuccess()) {
            return StmtCodegenResult("Array initializer element " + std::to_string(i) + " codegen failed") << elemResult;
        }

        ExprCodegenResult castResult = context.typeManager->typeCast(elemResult.getValue(), elemResult.getType(), elementTypeIdx, context.builder);
        if (false == castResult.isSuccess()) {
            return StmtCodegenResult("Type cast failed for array initializer element " + std::to_string(i)) << castResult;
        }

        context.builder.CreateStore(castResult.getValue(), elementPtr);
    }

    return StmtCodegenResult();
}

StmtCodegenResult NBlock::codegen(ASTContext &context) {
    llvm::Function *parentFunction = context.currentFunction->getFunction();
    block = llvm::BasicBlock::Create(context.llvmContext, name, parentFunction);

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
            TypeIdx paramTypeIdx = (param != nullptr) ? param->getTypeIdx() : InvalidTypeIdx;
            context.variableTable->insert(arg.getName().str(), std::make_pair(allocaInst, paramTypeIdx));
            if (param != nullptr) param = param->next;
        }
    }
    context.isInitializingFunction = false;

    for (NStatement *stmt = statements; stmt != nullptr; stmt = stmt->next) {
        if (nullptr != parent) {
            stmt->setParent(parent);
        }

        StmtCodegenResult stmtResult = stmt->codegen(context);
        if (false == stmtResult.isSuccess()) {
            return StmtCodegenResult("Failed to generate code for block statement") << stmtResult;
        }
    }


    if (nullptr != nextBlock && !context.builder.GetInsertBlock()->getTerminator()) {
        context.builder.CreateBr(nextBlock);
        context.builder.SetInsertPoint(nextBlock);
    }

    return StmtCodegenResult();
}

StmtCodegenResult NReturnStatement::codegen(ASTContext &context) {
    if (nullptr != expression) {
        ExprCodegenResult exprResult = expression->codegen(context);
        llvm::Value *value = exprResult.getValue();
        TypeIdx typeIdx = exprResult.getType();
        if (false == exprResult.isSuccess()) {
            return StmtCodegenResult("Failed to generate code for return expression") << exprResult;
        }
        ExprCodegenResult castResult = context.typeManager->typeCast(
            value,
            typeIdx,
            context.currentFunction->getReturnTypeIdx(),
            context.builder);
        if (false == castResult.isSuccess()) {
            return StmtCodegenResult("Type cast failed for return statement") << castResult;
        }
        context.builder.CreateRet(castResult.getValue());
    } else {
        context.builder.CreateRetVoid();
    }

    return StmtCodegenResult();
}

StmtCodegenResult NIfStatement::codegen(ASTContext &context) {
    llvm::Function *function = context.currentFunction->getFunction();
    llvm::BasicBlock *thenBlock = nullptr;
    llvm::BasicBlock *elseBlock = nullptr;
    llvm::BasicBlock *conditionBlock = llvm::BasicBlock::Create(context.llvmContext, "if_condition", function);
    llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(context.llvmContext, "if_merge", function);
    llvm::Value *conditionValue = nullptr;

    context.builder.CreateBr(conditionBlock);
    context.builder.SetInsertPoint(conditionBlock);
    ExprCodegenResult condResult = conditionNode->codegen(context);
    conditionValue = condResult.getValue();
    if (false == condResult.isSuccess()) {
        return StmtCodegenResult("Condition code generation failed for if statement") << condResult;
    }

    // Get the actual block after condition evaluation (may have changed due to short-circuit)
    llvm::BasicBlock *conditionEndBlock = context.builder.GetInsertBlock();

    thenBlockNode->setParent(parent);
    thenBlockNode->setName("if_then");
    thenBlockNode->setNextBlock(mergeBlock);
    StmtCodegenResult thenBlockResult = thenBlockNode->codegen(context);
    if (false == thenBlockResult.isSuccess()) {
        return StmtCodegenResult("Then block generation failed for if statement") << thenBlockResult;
    }
    thenBlock = thenBlockNode->getBlock();

    if (nullptr != elseBlockNode) {
        elseBlockNode->setParent(parent);
        elseBlockNode->setName("if_else");
        elseBlockNode->setNextBlock(mergeBlock);
        StmtCodegenResult elseBlockResult = elseBlockNode->codegen(context);
        if (false == elseBlockResult.isSuccess()) {
            return StmtCodegenResult("Else block generation failed for if statement") << elseBlockResult;
        }
        elseBlock = elseBlockNode->getBlock();
    }

    context.builder.SetInsertPoint(conditionEndBlock);
    context.builder.CreateCondBr(conditionValue, thenBlock, (nullptr != elseBlock) ? elseBlock : mergeBlock);
    context.builder.SetInsertPoint(mergeBlock);
    return StmtCodegenResult();
}

StmtCodegenResult NForStatement::codegen(ASTContext &context) {
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
    StmtCodegenResult bodyResult = bodyNode->codegen(context);
    if (false == bodyResult.isSuccess()) {
        return StmtCodegenResult("For loop body generation failed") << bodyResult;
    }
    loopBody = bodyNode->getBlock();

    incrementNode->codegen(context);
    context.builder.CreateBr(loopCondition);
    context.builder.SetInsertPoint(loopCondition);
    ExprCodegenResult condResult = conditionNode->codegen(context);
    llvm::Value *conditionValue = condResult.getValue();
    TypeIdx conditionTypeIdx = condResult.getType();
    if (false == condResult.isSuccess()) {
        return StmtCodegenResult("For loop condition generation failed") << condResult;
    }
    ExprCodegenResult conditionCastResult = context.typeManager->typeCast(
        conditionValue,
        conditionTypeIdx,
        context.typeManager->getPrimitiveIdx(VAR_TYPE_BOOL),
        context.builder);
    if (false == conditionCastResult.isSuccess()) {
        return StmtCodegenResult("Type cast failed for for loop condition") << conditionCastResult;
    }

    // Get the actual block after condition evaluation (may have changed due to short-circuit)
    llvm::BasicBlock *conditionEndBlock = context.builder.GetInsertBlock();

    if (nullptr == conditionValue) {
        context.builder.CreateBr(loopBody);
    } else {
        context.builder.SetInsertPoint(conditionEndBlock);
        context.builder.CreateCondBr(conditionValue, loopBody, afterBlock);
    }

    context.builder.SetInsertPoint(afterBlock);

    return StmtCodegenResult();
}

StmtCodegenResult NWhileStatement::codegen(ASTContext &context) {
    llvm::Function *function = context.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *loopCondition = llvm::BasicBlock::Create(context.llvmContext, "while_condition", function);
    llvm::BasicBlock *loopBody = nullptr;
    llvm::BasicBlock *afterBlock = llvm::BasicBlock::Create(context.llvmContext, "while_after", function);
    llvm::BasicBlock *previousBlock = context.builder.GetInsertBlock();

    context.builder.SetInsertPoint(loopCondition);
    ExprCodegenResult condResult = conditionNode->codegen(context);
    llvm::Value *conditionValue = condResult.getValue();
    TypeIdx conditionTypeIdx = condResult.getType();
    if (false == condResult.isSuccess()) {
        return StmtCodegenResult("While loop condition generation failed") << condResult;
    }
    ExprCodegenResult castResult = context.typeManager->typeCast(
        conditionValue,
        conditionTypeIdx,
        context.typeManager->getPrimitiveIdx(VAR_TYPE_BOOL),
        context.builder);
    if (false == castResult.isSuccess() || nullptr == castResult.getValue()) {
        return StmtCodegenResult("Type cast failed for while loop condition") << castResult;
    }
    conditionValue = castResult.getValue();

    // Get the actual block after condition evaluation (may have changed due to short-circuit)
    llvm::BasicBlock *conditionEndBlock = context.builder.GetInsertBlock();

    auto jumpGuard = toyc::utility::makeScopeGuard([&context]() {
        context.popJumpContext();
    });
    context.pushJumpContext(std::make_shared<NJumpContext>(loopCondition, afterBlock));

    bodyNode->setParent(this);
    bodyNode->setName("while_body");
    bodyNode->setNextBlock(loopCondition);
    StmtCodegenResult bodyResult = bodyNode->codegen(context);
    loopBody = bodyNode->getBlock();
    if (false == bodyResult.isSuccess() || nullptr == loopBody) {
        return StmtCodegenResult("While loop body generation failed") << bodyResult;
    }

    context.builder.SetInsertPoint(conditionEndBlock);
    context.builder.CreateCondBr(conditionValue, loopBody, afterBlock);

    context.builder.SetInsertPoint(previousBlock);
    if (true == isDoWhile) {
        context.builder.CreateBr(loopBody);
    } else {
        context.builder.CreateBr(loopCondition);
    }

    context.builder.SetInsertPoint(afterBlock);

    return StmtCodegenResult();
}

StmtCodegenResult NBreakStatement::codegen(ASTContext &context) {
    auto jumpCtx = context.getCurrentJumpContext();
    if (!jumpCtx) {
        return StmtCodegenResult("Break statement not within a loop or switch");
    }

    if (!jumpCtx->supportsBreak()) {
        return StmtCodegenResult("Break statement not supported in current context");
    }

    // Get the break target from the jump context
    llvm::BasicBlock *breakTarget = jumpCtx->getBreakTarget();
    context.builder.CreateBr(breakTarget);

    // Create an unreachable block after the break to avoid dangling insert point
    llvm::Function *function = context.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *unreachableBlock = llvm::BasicBlock::Create(
        context.llvmContext, "after_break", function);
    context.builder.SetInsertPoint(unreachableBlock);

    return StmtCodegenResult();
}

StmtCodegenResult NContinueStatement::codegen(ASTContext &context) {
    auto jumpCtx = context.getCurrentJumpContext();
    if (!jumpCtx) {
        return StmtCodegenResult("Continue statement not within a loop");
    }

    if (!jumpCtx->supportsContinue()) {
        return StmtCodegenResult("Continue statement not supported in " +
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

    return StmtCodegenResult();
}

StmtCodegenResult NLabelStatement::codegen(ASTContext &context) {
    llvm::Function *function = context.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *labelBlock = context.getLabel(label);

    if (labelBlock) {
        if (!labelBlock->empty()) {
            return StmtCodegenResult("Label '" + label + "' is already defined");
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
        StmtCodegenResult result = statement->codegen(context);
        if (!result.isSuccess()) {
            return result << StmtCodegenResult("Failed to generate code for statement after label");
        }
    }

    return StmtCodegenResult();
}

StmtCodegenResult NGotoStatement::codegen(ASTContext &context) {
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

    return StmtCodegenResult();
}

StmtCodegenResult NSwitchStatement::codegen(ASTContext &context) {
    ExprCodegenResult condResult = condition->codegen(context);
    llvm::Value *condValue = condResult.getValue();
    TypeIdx condTypeIdx = condResult.getType();

    if (false == condResult.isSuccess()) {
        return StmtCodegenResult("Failed to evaluate switch condition") << condResult;
    }

    TypeIdx intTypeIdx = context.typeManager->getPrimitiveIdx(VAR_TYPE_INT);
    ExprCodegenResult castResult = context.typeManager->typeCast(condValue, condTypeIdx, intTypeIdx, context.builder);
    if (false == castResult.isSuccess() || nullptr == castResult.getValue()) {
        return StmtCodegenResult("Failed to cast switch condition to integer") << castResult;
    }
    condValue = castResult.getValue();

    llvm::Function *function = context.currentFunction->getFunction();
    llvm::BasicBlock *currentBlock = context.builder.GetInsertBlock();
    llvm::BasicBlock *afterBlock = llvm::BasicBlock::Create(
        context.llvmContext, "switch_after", function);
    llvm::BasicBlock *switchBlock = llvm::BasicBlock::Create(
        context.llvmContext, "switch_entry", function);
    llvm::BasicBlock *defaultBlock = llvm::BasicBlock::Create(
        context.llvmContext, "switch_default", function);
    context.builder.SetInsertPoint(currentBlock);
    context.builder.CreateBr(switchBlock);
    context.builder.SetInsertPoint(switchBlock);

    llvm::SwitchInst *switchInst = context.builder.CreateSwitch(
        condValue,
        defaultBlock,
        10  // Estimated number of cases
    );

    auto jumpGuard = toyc::utility::makeScopeGuard([&context]() {
        context.popJumpContext();
    });
    context.pushJumpContext(std::make_shared<NSwitchContext>(afterBlock));

    // Store switch info in context for case statements to register themselves
    llvm::SwitchInst *oldSwitch = context.currentSwitch;
    llvm::BasicBlock *oldSwitchAfter = context.currentSwitchAfter;
    llvm::BasicBlock *oldDefaultBlock = context.currentSwitchDefault;
    bool oldHasDefault = context.switchHasDefault;
    auto restoreGuard = toyc::utility::makeScopeGuard([&context, oldSwitch, oldSwitchAfter, oldDefaultBlock, oldHasDefault]() {
        context.currentSwitch = oldSwitch;
        context.currentSwitchAfter = oldSwitchAfter;
        context.currentSwitchDefault = oldDefaultBlock;
        context.switchHasDefault = oldHasDefault;
    });
    context.currentSwitch = switchInst;
    context.currentSwitchAfter = afterBlock;
    context.currentSwitchDefault = defaultBlock;
    context.switchHasDefault = false;

    StmtCodegenResult bodyResult = body->codegen(context);
    if (false == bodyResult.isSuccess()) {
        return bodyResult << StmtCodegenResult("Failed to generate switch body");
    }

    llvm::BasicBlock *lastBlock = context.builder.GetInsertBlock();
    if (!lastBlock->getTerminator()) {
        context.builder.CreateBr(afterBlock);
    }

    // If no default case was found, make default block jump to after block
    if (!context.switchHasDefault) {
        context.builder.SetInsertPoint(defaultBlock);
        context.builder.CreateBr(afterBlock);
    }

    context.builder.SetInsertPoint(afterBlock);

    return StmtCodegenResult();
}

StmtCodegenResult NCaseStatement::codegen(ASTContext &context) {
    llvm::Function *function = context.currentFunction->getFunction();

    llvm::SwitchInst *switchInst = context.currentSwitch;
    if (!switchInst) {
        return StmtCodegenResult("Case statement outside of switch");
    }

    llvm::BasicBlock *caseBlock;

    if (isDefault) {
        // Use the pre-created default block
        caseBlock = context.currentSwitchDefault;
        context.switchHasDefault = true;
    } else {
        caseBlock = llvm::BasicBlock::Create(context.llvmContext, "switch_case", function);
    }

    // If current block doesn't have a terminator, branch to this case (fall-through)
    llvm::BasicBlock *currentBlock = context.builder.GetInsertBlock();
    if (currentBlock && !currentBlock->getTerminator()) {
        context.builder.CreateBr(caseBlock);
    }

    // Register the case with the switch instruction
    if (false == isDefault) {
        ExprCodegenResult valResult = value->codegen(context);
        llvm::Value *caseValue = valResult.getValue();
        if (false == valResult.isSuccess()) {
            return StmtCodegenResult("Failed to evaluate case value") << valResult;
        }

        llvm::ConstantInt *constInt = llvm::dyn_cast<llvm::ConstantInt>(caseValue);
        if (nullptr == constInt) {
            return StmtCodegenResult("Case value must be a constant integer");
        }

        switchInst->addCase(constInt, caseBlock);
    }

    context.builder.SetInsertPoint(caseBlock);

    return StmtCodegenResult();
}