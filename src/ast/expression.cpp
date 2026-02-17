#include "ast/expression.hpp"

#include <iostream>
#include <map>

#include "ast/external_definition.hpp"

using namespace toyc::ast;

ExprCodegenResult NLogicalOperator::codegen(ASTContext &context) {
    ExprCodegenResult lhsResult = lhs->codegen(context);
    llvm::Value *lhsValue = lhsResult.getValue();
    TypeIdx lhsTypeIdx = lhsResult.getType();

    if (false == lhsResult.isSuccess()) {
        return ExprCodegenResult("Failed to generate code for left operand in logical operation") << lhsResult;
    }

    TypeIdx boolIdx = context.typeManager->getPrimitiveIdx(VAR_TYPE_BOOL);

    CodegenResult castLhsResult = context.typeManager->typeCast(lhsValue, lhsTypeIdx, boolIdx, context.builder);
    if (false == castLhsResult.isSuccess()) {
        return ExprCodegenResult("Type cast failed for left-hand side in logical operation") << castLhsResult;
    }
    lhsValue = castLhsResult.getValue();

    llvm::Function *function = context.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *rhsBlock = llvm::BasicBlock::Create(context.llvmContext, "rhs", function);
    llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(context.llvmContext, "merge", function);
    llvm::BasicBlock *lhsEndBlock = context.builder.GetInsertBlock();

    llvm::Value *lhsCond =
        context.builder.CreateICmpNE(lhsValue, llvm::ConstantInt::get(lhsValue->getType(), 0), "lhs_cond");

    if (op == AND) {
        context.builder.CreateCondBr(lhsCond, rhsBlock, mergeBlock);
    } else {
        context.builder.CreateCondBr(lhsCond, mergeBlock, rhsBlock);
    }

    context.builder.SetInsertPoint(rhsBlock);
    CodegenResult rhsResult = rhs->codegen(context);
    llvm::Value *rhsValue = rhsResult.getValue();
    TypeIdx rhsTypeIdx = rhsResult.getType();

    if (false == rhsResult.isSuccess()) {
        return ExprCodegenResult("Failed to generate code for right operand in logical operation") << rhsResult;
    }

    CodegenResult castRhsResult = context.typeManager->typeCast(rhsValue, rhsTypeIdx, boolIdx, context.builder);
    if (false == castRhsResult.isSuccess()) {
        return ExprCodegenResult("Type cast failed for right-hand side in logical operation") << castRhsResult;
    }
    rhsValue = castRhsResult.getValue();

    llvm::BasicBlock *rhsEndBlock = context.builder.GetInsertBlock();
    context.builder.CreateBr(mergeBlock);

    context.builder.SetInsertPoint(mergeBlock);
    llvm::PHINode *phiNode = context.builder.CreatePHI(lhsValue->getType(), 2, "logical_result");
    phiNode->addIncoming(lhsValue, lhsEndBlock);
    phiNode->addIncoming(rhsValue, rhsEndBlock);

    return ExprCodegenResult(phiNode, boolIdx);
}

ExprCodegenResult NBinaryOperator::codegen(ASTContext &context) {
    CodegenResult rhsResult = rhs->codegen(context);
    ExprCodegenResult lhsResult = lhs->codegen(context);
    if (false == lhsResult.isSuccess() || false == rhsResult.isSuccess()) {
        return ExprCodegenResult("Failed to generate code for binary operator operands") << lhsResult << rhsResult;
    }

    llvm::Value *result = nullptr;
    llvm::Value *lhsValue = lhsResult.getValue();
    TypeIdx lhsTypeIdx = lhsResult.getType();
    llvm::Value *rhsValue = rhsResult.getValue();
    TypeIdx rhsTypeIdx = rhsResult.getType();

    TypeIdx targetTypeIdx = context.typeManager->getCommonTypeIdx(lhsTypeIdx, rhsTypeIdx);

    TypeIdx resultTypeIdx;
    if (op == EQ || op == NE || op == LE || op == GE || op == LT || op == GT) {
        resultTypeIdx = context.typeManager->getPrimitiveIdx(VAR_TYPE_BOOL);
    } else {
        resultTypeIdx = targetTypeIdx;
    }

    CodegenResult castLhsResult = context.typeManager->typeCast(lhsValue, lhsTypeIdx, targetTypeIdx, context.builder);
    if (false == castLhsResult.isSuccess()) {
        return ExprCodegenResult("Type cast failed for left-hand side in binary operation") << castLhsResult;
    }
    lhsValue = castLhsResult.getValue();

    CodegenResult castRhsResult = context.typeManager->typeCast(rhsValue, rhsTypeIdx, targetTypeIdx, context.builder);
    if (false == castRhsResult.isSuccess()) {
        return ExprCodegenResult("Type cast failed for right-hand side in binary operation") << castRhsResult;
    }
    rhsValue = castRhsResult.getValue();

    switch (op) {
        case ADD:
            if (context.typeManager->isFloatingPointType(targetTypeIdx))
                result = context.builder.CreateFAdd(lhsValue, rhsValue, "add");
            else
                result = context.builder.CreateAdd(lhsValue, rhsValue, "add");
            break;
        case SUB:
            if (context.typeManager->isFloatingPointType(targetTypeIdx))
                result = context.builder.CreateFSub(lhsValue, rhsValue, "sub");
            else
                result = context.builder.CreateSub(lhsValue, rhsValue, "sub");
            break;
        case MUL:
            if (context.typeManager->isFloatingPointType(targetTypeIdx))
                result = context.builder.CreateFMul(lhsValue, rhsValue, "mul");
            else
                result = context.builder.CreateMul(lhsValue, rhsValue, "mul");
            break;
        case DIV:
            if (context.typeManager->isFloatingPointType(targetTypeIdx))
                result = context.builder.CreateFDiv(lhsValue, rhsValue, "div");
            else
                result = context.builder.CreateSDiv(lhsValue, rhsValue, "div");
            break;
        case MOD:
            result = context.builder.CreateSRem(lhsValue, rhsValue, "mod");
            break;
        case LEFT:
            result = context.builder.CreateShl(lhsValue, rhsValue, "left");
            break;
        case RIGHT:
            result = context.builder.CreateLShr(lhsValue, rhsValue, "right");
            break;
        case EQ:
            result = context.typeManager->isFloatingPointType(targetTypeIdx)
                         ? context.builder.CreateFCmpOEQ(lhsValue, rhsValue, "eq")
                         : context.builder.CreateICmpEQ(lhsValue, rhsValue, "eq");
            break;
        case NE:
            result = context.typeManager->isFloatingPointType(targetTypeIdx)
                         ? context.builder.CreateFCmpONE(lhsValue, rhsValue, "ne")
                         : context.builder.CreateICmpNE(lhsValue, rhsValue, "ne");
            break;
        case LE:
            result = context.typeManager->isFloatingPointType(targetTypeIdx)
                         ? context.builder.CreateFCmpOLE(lhsValue, rhsValue, "le")
                         : context.builder.CreateICmpSLE(lhsValue, rhsValue, "le");
            break;
        case GE:
            result = context.typeManager->isFloatingPointType(targetTypeIdx)
                         ? context.builder.CreateFCmpOGE(lhsValue, rhsValue, "ge")
                         : context.builder.CreateICmpSGE(lhsValue, rhsValue, "ge");
            break;
        case LT:
            result = context.typeManager->isFloatingPointType(targetTypeIdx)
                         ? context.builder.CreateFCmpOLT(lhsValue, rhsValue, "lt")
                         : context.builder.CreateICmpSLT(lhsValue, rhsValue, "lt");
            break;
        case GT:
            result = context.typeManager->isFloatingPointType(targetTypeIdx)
                         ? context.builder.CreateFCmpOGT(lhsValue, rhsValue, "gt")
                         : context.builder.CreateICmpSGT(lhsValue, rhsValue, "gt");
            break;
        case BIT_AND:
            result = context.builder.CreateAnd(lhsValue, rhsValue, "bit_and");
            break;
        case BIT_OR:
            result = context.builder.CreateOr(lhsValue, rhsValue, "bit_or");
            break;
        case XOR:
            result = context.builder.CreateXor(lhsValue, rhsValue, "xor");
            break;
        default:
            return ExprCodegenResult("Unknown binary operator");
    }

    return ExprCodegenResult(result, resultTypeIdx);
}

ExprCodegenResult NUnaryExpression::codegen(ASTContext &context) {
    CodegenResult exprResult = expr->codegen(context);
    llvm::AllocaInst *allocaInst = nullptr;
    llvm::Value *one = context.builder.getInt32(1);
    llvm::Value *value = exprResult.getValue();
    TypeIdx typeIdx = exprResult.getType();

    if (false == exprResult.isSuccess()) {
        return ExprCodegenResult("Failed to generate code for unary expression operand") << exprResult;
    }

    if (op == L_INC || op == R_INC || op == L_DEC || op == R_DEC || op == ADDR || op == DEREF) {
        CodegenResult allocResult = expr->allocgen(context);
        if (false == allocResult.isSuccess()) {
            return ExprCodegenResult("Failed to get lvalue for unary increment/decrement") << allocResult;
        }
        allocaInst = static_cast<llvm::AllocaInst *>(allocResult.getAllocaInst());
    }

    if ((op == L_INC || op == R_INC || op == L_DEC || op == R_DEC) && context.typeManager->isConstQualified(typeIdx)) {
        return ExprCodegenResult("Increment/decrement of const-qualified variable");
    }

    bool isVolatile = context.typeManager->isVolatileQualified(typeIdx);
    llvm::Value *tmp = nullptr;
    switch (op) {
        case L_INC:
            value = context.builder.CreateAdd(value, one, "inc");
            context.builder.CreateStore(value, allocaInst, isVolatile);
            break;
        case R_INC:
            tmp = context.builder.CreateAdd(value, one, "inc");
            context.builder.CreateStore(tmp, allocaInst, isVolatile);
            break;
        case L_DEC:
            value = context.builder.CreateSub(value, one, "dec");
            context.builder.CreateStore(value, allocaInst, isVolatile);
            break;
        case R_DEC:
            tmp = context.builder.CreateSub(value, one, "dec");
            context.builder.CreateStore(tmp, allocaInst, isVolatile);
            break;
        case ADDR:
            value = allocaInst;
            break;
        case DEREF: {
            TypeIdx unqualifiedIdx = context.typeManager->unqualify(typeIdx);
            auto *ptrTc = dynamic_cast<const PointerTypeCodegen *>(context.typeManager->get(unqualifiedIdx));
            if (!ptrTc) {
                return ExprCodegenResult("Cannot dereference non-pointer type");
            }
            TypeIdx pointeeIdx = ptrTc->getPointeeIdx();
            bool pointeeIsVolatile = context.typeManager->isVolatileQualified(pointeeIdx);
            llvm::Type *pointeeType = context.typeManager->realize(pointeeIdx);
            value = context.builder.CreateLoad(pointeeType, value, pointeeIsVolatile, "deref");
        } break;
        case PLUS:
            break;
        case MINUS:
            value = context.builder.CreateNeg(value, "neg");
            break;
        case BIT_NOT:
            value = context.builder.CreateXor(value, context.builder.getInt32(-1), "bit_not");
            break;
        case LOG_NOT: {
            TypeIdx boolIdx = context.typeManager->getPrimitiveIdx(VAR_TYPE_BOOL);
            CodegenResult castResult = context.typeManager->typeCast(value, typeIdx, boolIdx, context.builder);
            if (false == castResult.isSuccess()) {
                return ExprCodegenResult("Failed to cast value to bool for logical not") << castResult;
            }
            value = context.builder.CreateICmpEQ(value, context.builder.getFalse(), "log_not");
        } break;
        default:
            return ExprCodegenResult("Unknown unary operator");
    }

    TypeIdx resultTypeIdx;
    if (op == ADDR) {
        resultTypeIdx = context.typeManager->getPointerIdx(typeIdx, 1);
    } else if (op == DEREF) {
        TypeIdx unqualifiedIdx = context.typeManager->unqualify(typeIdx);
        auto *ptrTc = dynamic_cast<const PointerTypeCodegen *>(context.typeManager->get(unqualifiedIdx));
        resultTypeIdx = ptrTc ? ptrTc->getPointeeIdx() : InvalidTypeIdx;
    } else if (op == LOG_NOT) {
        resultTypeIdx = context.typeManager->getPrimitiveIdx(VAR_TYPE_BOOL);
    } else {
        resultTypeIdx = typeIdx;
    }

    return ExprCodegenResult(value, resultTypeIdx);
}

ExprCodegenResult NConditionalExpression::codegen(ASTContext &context) {
    CodegenResult condResult = condition->codegen(context);
    llvm::Value *condValue = condResult.getValue();
    if (false == condResult.isSuccess()) {
        return ExprCodegenResult("Condition expression code generation failed") << condResult;
    }

    condValue = context.builder.CreateICmpNE(condValue, llvm::ConstantInt::get(condValue->getType(), 0), "cond");

    llvm::Function *function = context.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *trueBlock = llvm::BasicBlock::Create(context.llvmContext, "true", function);
    llvm::BasicBlock *falseBlock = llvm::BasicBlock::Create(context.llvmContext, "false", function);
    llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(context.llvmContext, "merge", function);

    context.builder.CreateCondBr(condValue, trueBlock, falseBlock);

    context.builder.SetInsertPoint(trueBlock);
    CodegenResult trueResult = trueExpr->codegen(context);
    llvm::Value *trueValue = trueResult.getValue();
    TypeIdx trueTypeIdx = trueResult.getType();
    if (false == trueResult.isSuccess()) {
        return ExprCodegenResult("True expression code generation failed") << trueResult;
    }
    llvm::BasicBlock *trueEndBlock = context.builder.GetInsertBlock();
    context.builder.CreateBr(mergeBlock);

    context.builder.SetInsertPoint(falseBlock);
    CodegenResult falseResult = falseExpr->codegen(context);
    llvm::Value *falseValue = falseResult.getValue();
    if (false == falseResult.isSuccess()) {
        return ExprCodegenResult("False expression code generation failed") << falseResult;
    }
    llvm::BasicBlock *falseEndBlock = context.builder.GetInsertBlock();
    context.builder.CreateBr(mergeBlock);

    context.builder.SetInsertPoint(mergeBlock);
    llvm::PHINode *phiNode = context.builder.CreatePHI(trueValue->getType(), 2, "phi");
    phiNode->addIncoming(trueValue, trueEndBlock);
    phiNode->addIncoming(falseValue, falseEndBlock);

    return ExprCodegenResult(phiNode, trueTypeIdx);
}

ExprCodegenResult NIdentifier::codegen(ASTContext &context) {
    auto [isFound, variablePair] = context.variableTable->lookup(name);
    auto [allocaInst, typeIdx] = variablePair;
    llvm::Value *value = nullptr;
    if (false == isFound || nullptr == allocaInst) {
        return ExprCodegenResult("Variable not found: " + name);
    }

    llvm::Type *type = context.typeManager->realize(typeIdx);

    if (nullptr != type && true == type->isArrayTy()) {
        std::vector<llvm::Value *> indices(2);
        indices[0] = llvm::ConstantInt::get(context.llvmContext, llvm::APInt(32, 0));
        indices[1] = llvm::ConstantInt::get(context.llvmContext, llvm::APInt(32, 0));

        value = context.builder.CreateGEP(allocaInst->getAllocatedType(), allocaInst, indices, name + "_decay");

        auto *arrTc = dynamic_cast<const ArrayTypeCodegen *>(context.typeManager->get(typeIdx));
        TypeIdx elementIdx = arrTc ? arrTc->getElementIdx() : InvalidTypeIdx;
        return ExprCodegenResult(value, context.typeManager->getPointerIdx(elementIdx, 1));
    }

    bool isVolatile = context.typeManager->isVolatileQualified(typeIdx);
    value = context.builder.CreateLoad(allocaInst->getAllocatedType(), allocaInst, isVolatile, name);
    if (nullptr == value) {
        return ExprCodegenResult("Load failed for variable: " + name);
    }

    return ExprCodegenResult(value, typeIdx);
}

AllocCodegenResult NIdentifier::allocgen(ASTContext &context) {
    auto [isFound, variablePair] = context.variableTable->lookup(name);
    auto [allocaInst, typeIdx] = variablePair;

    if (false == isFound || nullptr == allocaInst) {
        return AllocCodegenResult("Variable not found: " + name);
    }

    return AllocCodegenResult(allocaInst, typeIdx);
}

ExprCodegenResult NAssignment::codegen(ASTContext &context) {
    AllocCodegenResult lhsResult = lhs->allocgen(context);
    llvm::AllocaInst *lhsAlloca = static_cast<llvm::AllocaInst *>(lhsResult.getAllocaInst());
    TypeIdx lhsTypeIdx = lhsResult.getType();
    CodegenResult rhsResult = rhs->codegen(context);
    llvm::Value *rhsValue = rhsResult.getValue();
    TypeIdx rhsTypeIdx = rhsResult.getType();
    if (false == rhsResult.isSuccess() || false == lhsResult.isSuccess()) {
        return ExprCodegenResult("Assignment failed due to null values") << lhsResult << rhsResult;
    }

    if (context.typeManager->isConstQualified(lhsTypeIdx)) {
        return ExprCodegenResult("Assignment to const-qualified variable");
    }

    CodegenResult castResult = context.typeManager->typeCast(rhsValue, rhsTypeIdx, lhsTypeIdx, context.builder);
    if (false == castResult.isSuccess()) {
        return ExprCodegenResult("Type cast failed during assignment") << castResult;
    }
    rhsValue = castResult.getValue();
    bool isVolatile = context.typeManager->isVolatileQualified(lhsTypeIdx);
    context.builder.CreateStore(rhsValue, lhsAlloca, isVolatile);
    return ExprCodegenResult(rhsValue, lhsTypeIdx);
}

ExprCodegenResult NArguments::codegen(ASTContext &context) {
    return expr->codegen(context);
}

ExprCodegenResult NFunctionCall::codegen(ASTContext &context) {
    std::vector<llvm::Value *> args;
    NFunctionDefinition *function = context.functionDefinitions[name];
    llvm::Value *res = nullptr;
    if (nullptr == function) {
        return ExprCodegenResult("Function not found: " + name);
    }

    NParameter *paramIt = function->getParams();
    NArguments *argNode = argNodes;

    for (; argNode != nullptr; argNode = argNode->next) {
        CodegenResult argResult = argNode->codegen(context);
        llvm::Value *argValue = argResult.getValue();
        TypeIdx argTypeIdx = argResult.getType();
        if (false == argResult.isSuccess()) {
            return ExprCodegenResult("Argument code generation failed for function call: " + name) << argResult;
        }

        if (paramIt != nullptr && false == paramIt->isVariadic) {
            CodegenResult castResult =
                context.typeManager->typeCast(argValue, argTypeIdx, paramIt->getTypeIdx(), context.builder);
            if (false == castResult.isSuccess()) {
                return ExprCodegenResult("Type cast failed for argument in function call: " + name) << castResult;
            }
            argValue = castResult.getValue();
        }

        if (paramIt != nullptr) {
            paramIt = paramIt->next;
        }

        args.push_back(argValue);
    }

    res = context.builder.CreateCall(function->getFunction(), args);

    return ExprCodegenResult(res, function->getReturnTypeIdx());
}

ExprCodegenResult NMemberAccess::codegen(ASTContext &context) {
    CodegenResult allocResult = allocgen(context);
    if (false == allocResult.isSuccess()) {
        return ExprCodegenResult("Member access allocation failed for member: " + memberName) << allocResult;
    }

    llvm::Value *memberPtr = allocResult.getAllocaInst();
    TypeIdx memberTypeIdx = allocResult.getType();
    llvm::Type *memberType = context.typeManager->realize(memberTypeIdx);
    llvm::Value *memberValue = context.builder.CreateLoad(memberType, memberPtr, "member_value");

    return ExprCodegenResult(memberValue, memberTypeIdx);
}

AllocCodegenResult NMemberAccess::allocgen(ASTContext &context) {
    AllocCodegenResult baseResult = base->allocgen(context);
    llvm::Value *baseValue = baseResult.getAllocaInst();
    TypeIdx baseTypeIdx = baseResult.getType();

    if (false == baseResult.isSuccess()) {
        return AllocCodegenResult("Base expression code generation failed for member access: " + memberName)
               << baseResult;
    }

    if (true == isPointerAccess) {
        auto *ptrTc = dynamic_cast<const PointerTypeCodegen *>(context.typeManager->get(baseTypeIdx));
        if (!ptrTc) {
            return AllocCodegenResult("Failed to get pointee type for dereferencing in member access");
        }
        TypeIdx derefTypeIdx = ptrTc->getPointeeIdx();
        llvm::Type *derefType = context.typeManager->realize(derefTypeIdx);
        baseValue = context.builder.CreateLoad(derefType, baseValue, "deref_base");
        baseTypeIdx = derefTypeIdx;
    }

    auto *structTc = dynamic_cast<const StructTypeCodegen *>(context.typeManager->get(baseTypeIdx));
    if (!structTc) {
        return AllocCodegenResult("Base type is not a struct for member access: " + memberName);
    }

    int memberIndex = structTc->getMemberIndex(memberName);
    if (-1 == memberIndex) {
        return AllocCodegenResult("Member not found in struct: " + memberName);
    }

    TypeIdx memberTypeIdx = structTc->getMemberTypeIdx(memberIndex);

    std::vector<llvm::Value *> indices;
    indices.push_back(context.builder.getInt32(0));
    indices.push_back(context.builder.getInt32(memberIndex));

    llvm::Type *structType = context.typeManager->realize(baseTypeIdx);
    llvm::Value *memberPtr = context.builder.CreateGEP(structType, baseValue, indices, "member_ptr");

    return AllocCodegenResult(memberPtr, memberTypeIdx);
}

ExprCodegenResult NInteger::codegen(ASTContext &context) {
    return ExprCodegenResult(context.builder.getInt32(value), context.typeManager->getPrimitiveIdx(VAR_TYPE_INT));
}

ExprCodegenResult NFloat::codegen(ASTContext &context) {
    return ExprCodegenResult(llvm::ConstantFP::get(llvm::Type::getDoubleTy(context.llvmContext), value),
                             context.typeManager->getPrimitiveIdx(VAR_TYPE_DOUBLE));
}

ExprCodegenResult NString::codegen(ASTContext &context) {
    TypeIdx charIdx = context.typeManager->getPrimitiveIdx(VAR_TYPE_CHAR);
    return ExprCodegenResult(context.builder.CreateGlobalStringPtr(value, "string_literal"),
                             context.typeManager->getPointerIdx(charIdx, 1));
}

CodegenResult<llvm::Value *> NDeclarator::getArraySizeValue(ASTContext &context) {
    if (arrayDimensions.empty()) {
        return CodegenResult<llvm::Value *>("No array dimensions in declarator");
    }

    ExprCodegenResult sizeResult = arrayDimensions[0]->codegen(context);
    if (false == sizeResult.isSuccess()) {
        return CodegenResult<llvm::Value *>("Failed to generate code for array size expression") << sizeResult;
    }
    for (size_t i = 1; i < arrayDimensions.size(); ++i) {
        sizeResult = NBinaryOperator(arrayDimensions[i - 1], BineryOperator::MUL, arrayDimensions[i]).codegen(context);

        if (false == sizeResult.isSuccess()) {
            return CodegenResult<llvm::Value *>("Failed to compute total size for multi-dimensional array")
                   << sizeResult;
        }
    }

    return CodegenResult<llvm::Value *>(sizeResult.getValue());
}

ExprCodegenResult NArraySubscript::codegen(ASTContext &context) {
    CodegenResult ptrResult = allocgen(context);
    if (false == ptrResult.isSuccess()) {
        return ExprCodegenResult("Failed to generate pointer for array subscript") << ptrResult;
    }

    TypeIdx elemTypeIdx = ptrResult.getType();
    llvm::Type *elemType = context.typeManager->realize(elemTypeIdx);
    llvm::Value *value = context.builder.CreateLoad(elemType, ptrResult.getAllocaInst(), "arrayelem");

    return ExprCodegenResult(value, elemTypeIdx);
}

AllocCodegenResult NArraySubscript::allocgen(ASTContext &context) {
    CodegenResult indexResult = index->codegen(context);
    if (false == indexResult.isSuccess()) {
        return AllocCodegenResult("Failed to generate code for array index expression") << indexResult;
    }

    CodegenResult arrayResult = array->allocgen(context);
    if (false == arrayResult.isSuccess()) {
        return AllocCodegenResult("Failed to generate lvalue for array in subscript operation") << arrayResult;
    }

    llvm::Value *basePtr = arrayResult.getAllocaInst();
    TypeIdx arrayTypeIdx = arrayResult.getType();
    llvm::Type *arrayType = context.typeManager->realize(arrayTypeIdx);

    if (auto *arrTc = dynamic_cast<const ArrayTypeCodegen *>(context.typeManager->get(arrayTypeIdx))) {
        TypeIdx elementTypeIdx = arrTc->getElementIdx();
        llvm::Type *elementType = context.typeManager->realize(elementTypeIdx);
        std::vector<llvm::Value *> indices(2);
        indices[0] = context.builder.getInt32(0);
        indices[1] = indexResult.getValue();

        llvm::Value *elementPtr = context.builder.CreateGEP(arrayType, basePtr, indices, "arrayidx");

        return AllocCodegenResult(elementPtr, elementTypeIdx);
    } else if (auto *ptrTc = dynamic_cast<const PointerTypeCodegen *>(context.typeManager->get(arrayTypeIdx))) {
        TypeIdx elementTypeIdx = ptrTc->getPointeeIdx();
        llvm::Type *elementType = context.typeManager->realize(elementTypeIdx);

        llvm::Value *ptrValue = context.builder.CreateLoad(arrayType, basePtr, "load_ptr");
        llvm::Value *elementPtr = context.builder.CreateGEP(elementType, ptrValue, indexResult.getValue(), "ptridx");

        return AllocCodegenResult(elementPtr, elementTypeIdx);
    }

    std::string typeName = context.typeManager->getTypeName(arrayType);
    return AllocCodegenResult("Base is not an array type in subscript operation, got: " + typeName);
}

ExprCodegenResult NInitializerList::codegen(ASTContext &context) {
    return ExprCodegenResult("InitializerList cannot be used directly in expressions");
}

ExprCodegenResult NCastExpression::codegen(ASTContext &context) {
    ExprCodegenResult exprResult = expr->codegen(context);
    if (!exprResult.isSuccess()) {
        return ExprCodegenResult("Failed to generate code for cast expression") << exprResult;
    }

    if (targetTypeIdx == InvalidTypeIdx) {
        return ExprCodegenResult("Target type is invalid in cast expression");
    }

    TypeIdx sourceTypeIdx = exprResult.getType();
    llvm::Value *sourceValue = exprResult.getValue();

    CodegenResult castResult =
        context.typeManager->typeCast(sourceValue, sourceTypeIdx, targetTypeIdx, context.builder);
    if (!castResult.isSuccess()) {
        return ExprCodegenResult("Type cast failed in cast expression") << castResult;
    }

    return ExprCodegenResult(castResult.getValue(), targetTypeIdx);
}

ExprCodegenResult NSizeofExpression::codegen(ASTContext &context) {
    llvm::Type *type = nullptr;

    if (true == isSizeofType) {
        type = context.typeManager->realize(targetTypeIdx);
        if (nullptr == type) {
            return ExprCodegenResult("Target type is null in sizeof expression");
        }
    } else {
        AllocCodegenResult allocResult = expr->allocgen(context);
        if (allocResult.isSuccess() && allocResult.getType() != InvalidTypeIdx) {
            type = context.typeManager->realize(allocResult.getType());
        } else {
            ExprCodegenResult exprResult = expr->codegen(context);
            if (!exprResult.isSuccess()) {
                return ExprCodegenResult("Failed to generate code for sizeof expression") << exprResult;
            }
            type = context.typeManager->realize(exprResult.getType());
        }
    }

    llvm::DataLayout dataLayout(&(context.module));
    uint64_t sizeInBytes = dataLayout.getTypeAllocSize(type);

    llvm::Value *sizeValue = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.llvmContext), sizeInBytes);

    return ExprCodegenResult(sizeValue, context.typeManager->getPrimitiveIdx(VAR_TYPE_INT));
}

ExprCodegenResult NCommaExpression::codegen(ASTContext &context) {
    ExprCodegenResult leftResult = left->codegen(context);
    if (!leftResult.isSuccess()) {
        return ExprCodegenResult("Failed to generate code for left operand of comma expression") << leftResult;
    }

    ExprCodegenResult rightResult = right->codegen(context);
    if (!rightResult.isSuccess()) {
        return ExprCodegenResult("Failed to generate code for right operand of comma expression") << rightResult;
    }

    return rightResult;
}
