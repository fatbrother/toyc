#include "ast/expression.hpp"
#include "ast/external_definition.hpp"

#include <iostream>
#include <map>

using namespace toyc::ast;

ExprCodegenResult NLogicalOperator::codegen(ASTContext &context) {
    // Evaluate left operand
    ExprCodegenResult lhsResult = lhs->codegen(context);
    llvm::Value *lhsValue = lhsResult.getValue();
    llvm::Type* lhsType = lhsResult.getType();

    if (false == lhsResult.isSuccess()) {
        return ExprCodegenResult("Failed to generate code for left operand in logical operation") << lhsResult;
    }

    // Cast lhs to bool
    CodegenResult castLhsResult = typeCast(lhsValue, lhsType, context.typeManager->getBoolType(), context);
    if (false == castLhsResult.isSuccess()) {
        return ExprCodegenResult("Type cast failed for left-hand side in logical operation") << castLhsResult;
    }
    lhsValue = castLhsResult.getValue();

    // Create basic blocks for short-circuit evaluation
    llvm::Function *function = context.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *rhsBlock = llvm::BasicBlock::Create(context.llvmContext, "rhs", function);
    llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(context.llvmContext, "merge", function);
    llvm::BasicBlock *lhsEndBlock = context.builder.GetInsertBlock();

    // Convert to i1 for conditional branch
    llvm::Value *lhsCond = context.builder.CreateICmpNE(
        lhsValue,
        llvm::ConstantInt::get(lhsValue->getType(), 0),
        "lhs_cond");

    // Create conditional branch based on operator
    if (op == AND) {
        // AND: if lhs is false, skip rhs evaluation
        context.builder.CreateCondBr(lhsCond, rhsBlock, mergeBlock);
    } else { // OR
        // OR: if lhs is true, skip rhs evaluation
        context.builder.CreateCondBr(lhsCond, mergeBlock, rhsBlock);
    }

    // Generate rhs block
    context.builder.SetInsertPoint(rhsBlock);
    CodegenResult rhsResult = rhs->codegen(context);
    llvm::Value *rhsValue = rhsResult.getValue();
    llvm::Type* rhsType = rhsResult.getType();

    if (false == rhsResult.isSuccess()) {
        return ExprCodegenResult("Failed to generate code for right operand in logical operation") << rhsResult;
    }

    // Cast rhs to bool
    CodegenResult castRhsResult = typeCast(rhsValue, rhsType, context.typeManager->getBoolType(), context);
    if (false == castRhsResult.isSuccess()) {
        return ExprCodegenResult("Type cast failed for right-hand side in logical operation") << castRhsResult;
    }
    rhsValue = castRhsResult.getValue();

    llvm::BasicBlock *rhsEndBlock = context.builder.GetInsertBlock();
    context.builder.CreateBr(mergeBlock);

    // Merge block with PHI node
    context.builder.SetInsertPoint(mergeBlock);
    llvm::PHINode *phiNode = context.builder.CreatePHI(lhsValue->getType(), 2, "logical_result");
    phiNode->addIncoming(lhsValue, lhsEndBlock);
    phiNode->addIncoming(rhsValue, rhsEndBlock);

    return ExprCodegenResult(phiNode, context.typeManager->getBoolType());
}

ExprCodegenResult NBinaryOperator::codegen(ASTContext &context) {
    CodegenResult rhsResult = rhs->codegen(context);
    ExprCodegenResult lhsResult = lhs->codegen(context);
    if (false == lhsResult.isSuccess() || false == rhsResult.isSuccess()) {
        return ExprCodegenResult("Failed to generate code for binary operator operands")
            << lhsResult << rhsResult;
    }

    llvm::Value *result = nullptr;
    llvm::Type* resultType = nullptr;
    llvm::Type* targetType = nullptr;
    llvm::Value *lhsValue = lhsResult.getValue();
    llvm::Type* lhsType = lhsResult.getType();
    llvm::Value *rhsValue = rhsResult.getValue();
    llvm::Type* rhsType = rhsResult.getType();

    targetType = context.typeManager->getCommonType(lhsType, rhsType);
    if (op == EQ || op == NE || op == LE || op == GE || op == LT || op == GT) {
        resultType = context.typeManager->getBoolType();
    } else {
        resultType = targetType;
    }

    CodegenResult castLhsResult = typeCast(
        lhsValue,
        lhsType,
        targetType,
        context);
    if (false == castLhsResult.isSuccess()) {
        return ExprCodegenResult("Type cast failed for left-hand side in binary operation") << castLhsResult;
    }
    lhsValue = castLhsResult.getValue();

    CodegenResult castRhsResult = typeCast(
        rhsValue,
        rhsType,
        targetType,
        context);
    if (false == castRhsResult.isSuccess()) {
        return ExprCodegenResult("Type cast failed for right-hand side in binary operation") << castRhsResult;
    }
    rhsValue = castRhsResult.getValue();

    switch (op) {
        case ADD:
            if (isFloatingPointType(targetType))
                result = context.builder.CreateFAdd(lhsValue, rhsValue, "add");
            else
                result = context.builder.CreateAdd(lhsValue, rhsValue, "add");
            break;
        case SUB:
            if (isFloatingPointType(targetType))
                result = context.builder.CreateFSub(lhsValue, rhsValue, "sub");
            else
                result = context.builder.CreateSub(lhsValue, rhsValue, "sub");
            break;
        case MUL:
            if (isFloatingPointType(targetType))
                result = context.builder.CreateFMul(lhsValue, rhsValue, "mul");
            else
                result = context.builder.CreateMul(lhsValue, rhsValue, "mul");
            break;
        case DIV:
            if (isFloatingPointType(targetType))
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
            result = isFloatingPointType(targetType) ?
                     context.builder.CreateFCmpOEQ(lhsValue, rhsValue, "eq") :
                     context.builder.CreateICmpEQ(lhsValue, rhsValue, "eq");
            break;
        case NE:
            result = isFloatingPointType(targetType) ?
                     context.builder.CreateFCmpONE(lhsValue, rhsValue, "ne") :
                     context.builder.CreateICmpNE(lhsValue, rhsValue, "ne");
            break;
        case LE:
            result = isFloatingPointType(targetType) ?
                     context.builder.CreateFCmpOLE(lhsValue, rhsValue, "le") :
                     context.builder.CreateICmpSLE(lhsValue, rhsValue, "le");
            break;
        case GE:
            result = isFloatingPointType(targetType) ?
                     context.builder.CreateFCmpOGE(lhsValue, rhsValue, "ge") :
                     context.builder.CreateICmpSGE(lhsValue, rhsValue, "ge");
            break;
        case LT:
            result = isFloatingPointType(targetType) ?
                     context.builder.CreateFCmpOLT(lhsValue, rhsValue, "lt") :
                     context.builder.CreateICmpSLT(lhsValue, rhsValue, "lt");
            break;
        case GT:
            result = isFloatingPointType(targetType) ?
                     context.builder.CreateFCmpOGT(lhsValue, rhsValue, "gt") :
                     context.builder.CreateICmpSGT(lhsValue, rhsValue, "gt");
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

    return ExprCodegenResult(result, resultType);
}

ExprCodegenResult NUnaryExpression::codegen(ASTContext &context) {
    CodegenResult exprResult = expr->codegen(context);
    llvm::AllocaInst *allocaInst = nullptr;
    llvm::Value *one = context.builder.getInt32(1);
    llvm::Value *value = exprResult.getValue();
    llvm::Type* type = exprResult.getType();
    llvm::Type* resultType = nullptr;

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

    llvm::Value *tmp = nullptr;
    switch (op) {
        case L_INC:
            value = context.builder.CreateAdd(value, one, "inc");
            context.builder.CreateStore(value, allocaInst);
            break;
        case R_INC:
            tmp = context.builder.CreateAdd(value, one, "inc");
            context.builder.CreateStore(tmp, allocaInst);
            break;
        case L_DEC:
            value = context.builder.CreateSub(value, one, "dec");
            context.builder.CreateStore(value, allocaInst);
            break;
        case R_DEC:
            tmp = context.builder.CreateSub(value, one, "dec");
            context.builder.CreateStore(tmp, allocaInst);
            break;
        case ADDR:
            value = allocaInst;
            break;
        case DEREF: {
                llvm::Type* pointeeType = context.typeManager->getPointeeType(type);
                if (nullptr == pointeeType) {
                    return ExprCodegenResult("Cannot dereference non-pointer type");
                }

                value = context.builder.CreateLoad(pointeeType, value, "deref");
            }
            break;
        case PLUS:
            break;
        case MINUS:
            value = context.builder.CreateNeg(value, "neg");
            break;
        case BIT_NOT:
            value = context.builder.CreateXor(value, context.builder.getInt32(-1), "bit_not");
            break;
        case LOG_NOT: {
                CodegenResult castResult = typeCast(value, type, context.typeManager->getBoolType(), context);
                if (false == castResult.isSuccess()) {
                    return ExprCodegenResult("Failed to cast value to bool for logical not") << castResult;
                }
                value = context.builder.CreateICmpEQ(value, context.builder.getFalse(), "log_not");
            }
            break;
        default:
            return ExprCodegenResult("Unknown unary operator");
    }

    if (op == ADDR) {
        resultType = context.typeManager->getPointerType(type);
    } else if (op == DEREF) {
        resultType = context.typeManager->getPointeeType(type);
    } else if (op == LOG_NOT) {
        resultType = context.typeManager->getBoolType();
    } else {
        resultType = type;
    }

    return ExprCodegenResult(value, resultType);
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

    // Generate true branch
    context.builder.SetInsertPoint(trueBlock);
    CodegenResult trueResult = trueExpr->codegen(context);
    llvm::Value *trueValue = trueResult.getValue();
    llvm::Type* trueType = trueResult.getType();
    if (false == trueResult.isSuccess()) {
        return ExprCodegenResult("True expression code generation failed") << trueResult;
    }
    // Get the actual block after codegen (it might have changed due to nested expressions)
    llvm::BasicBlock *trueEndBlock = context.builder.GetInsertBlock();
    context.builder.CreateBr(mergeBlock);

    // Generate false branch
    context.builder.SetInsertPoint(falseBlock);
    CodegenResult falseResult = falseExpr->codegen(context);
    llvm::Value *falseValue = falseResult.getValue();
    llvm::Type* falseType = falseResult.getType();
    if (false == falseResult.isSuccess()) {
        return ExprCodegenResult("False expression code generation failed") << falseResult;
    }
    // Get the actual block after codegen (it might have changed due to nested expressions)
    llvm::BasicBlock *falseEndBlock = context.builder.GetInsertBlock();
    context.builder.CreateBr(mergeBlock);

    context.builder.SetInsertPoint(mergeBlock);

    // Merge the two branches using the actual ending blocks
    llvm::PHINode * phiNode = context.builder.CreatePHI(trueValue->getType(), 2, "phi");

    phiNode->addIncoming(trueValue, trueEndBlock);
    phiNode->addIncoming(falseValue, falseEndBlock);

    return ExprCodegenResult(phiNode, trueType);
}

ExprCodegenResult NIdentifier::codegen(ASTContext &context) {
    auto [isFound, variablePair] = context.variableTable->lookup(name);
    auto [allocaInst, type] = variablePair;
    llvm::Value *value = nullptr;
    if (false == isFound || nullptr == allocaInst) {
        return ExprCodegenResult("Variable not found: " + name);
    }

    if (nullptr != type && true == type->isArrayTy()) {
        std::vector<llvm::Value*> indices(2);
        indices[0] = llvm::ConstantInt::get(context.llvmContext, llvm::APInt(32, 0));
        indices[1] = llvm::ConstantInt::get(context.llvmContext, llvm::APInt(32, 0));

        value = context.builder.CreateGEP(
            allocaInst->getAllocatedType(),
            allocaInst,
            indices,
            name + "_decay"
        );

        llvm::Type* elementType = context.typeManager->getArrayElementType(type);
        return ExprCodegenResult(value, context.typeManager->getPointerType(elementType));
    }

    value = context.builder.CreateLoad(allocaInst->getAllocatedType(), allocaInst, name);
    if (nullptr == value) {
        return ExprCodegenResult("Load failed for variable: " + name);
    }

    return ExprCodegenResult(value, type);
}

AllocCodegenResult NIdentifier::allocgen(ASTContext &context) {
    auto [isFound, variablePair] = context.variableTable->lookup(name);
    auto [allocaInst, type] = variablePair;

    if (false == isFound || nullptr == allocaInst) {
        return AllocCodegenResult("Variable not found: " + name);
    }

    return AllocCodegenResult(allocaInst, type);
}

ExprCodegenResult NAssignment::codegen(ASTContext &context) {
    AllocCodegenResult lhsResult = lhs->allocgen(context);
    llvm::AllocaInst *lhsAlloca = static_cast<llvm::AllocaInst *>(lhsResult.getAllocaInst());
    llvm::Type* lhsType = lhsResult.getType();
    CodegenResult rhsResult = rhs->codegen(context);
    llvm::Value *rhsValue = rhsResult.getValue();
    llvm::Type* rhsType = rhsResult.getType();
    if (false == rhsResult.isSuccess() || false == lhsResult.isSuccess()) {
        return ExprCodegenResult("Assignment failed due to null values") << lhsResult << rhsResult;
    }

    CodegenResult castResult = typeCast(
        rhsValue,
        rhsType,
        lhsType,
        context);
    if (false == castResult.isSuccess()) {
        return ExprCodegenResult("Type cast failed during assignment") << castResult;
    }
    rhsValue = castResult.getValue();
    context.builder.CreateStore(rhsValue, lhsAlloca);
    return ExprCodegenResult(rhsValue, lhsType);
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
        llvm::Type* argType = argResult.getType();
        if (false == argResult.isSuccess()) {
            return ExprCodegenResult("Argument code generation failed for function call: " + name) << argResult;
        }

        if (paramIt != nullptr && false == paramIt->isVariadic) {
            CodegenResult castResult = typeCast(
                argValue,
                argType,
                context.typeManager->realize(paramIt->getTypeDescriptor(), context),
                context);
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

    return ExprCodegenResult(res, function->getReturnType());
}

ExprCodegenResult NMemberAccess::codegen(ASTContext &context) {
    CodegenResult allocResult = allocgen(context);
    if (false == allocResult.isSuccess()) {
        return ExprCodegenResult("Member access allocation failed for member: " + memberName) << allocResult;
    }

    llvm::Value *memberPtr = allocResult.getAllocaInst();
    llvm::Type* memberType = allocResult.getType();
    llvm::Value *memberValue = context.builder.CreateLoad(
        memberType,
        memberPtr,
        "member_value");

    return ExprCodegenResult(memberValue, memberType);
}

AllocCodegenResult NMemberAccess::allocgen(ASTContext &context) {
    AllocCodegenResult baseResult = base->allocgen(context);
    llvm::Value *baseValue = baseResult.getAllocaInst();
    llvm::Type* baseType = baseResult.getType();
    llvm::Type *childLLVMType = nullptr;

    if (false == baseResult.isSuccess()) {
        return AllocCodegenResult("Base expression code generation failed for member access: " + memberName) << baseResult;
    }

    if (true == baseType->isPointerTy() && true == isPointerAccess) {
        llvm::Type* derefType = context.typeManager->getPointeeType(baseType);
        if (nullptr == derefType) {
            return AllocCodegenResult("Failed to get pointee type for dereferencing in member access");
        }
        baseValue = context.builder.CreateLoad(
            derefType,
            baseValue,
            "deref_base");
        baseType = derefType;
    }

    if (false == baseType->isStructTy()) {
        return AllocCodegenResult("Base type is not a struct for member access: " + memberName);
    }

    auto* structType = llvm::cast<llvm::StructType>(baseType);
    const auto metadata = context.typeManager->getStructMetadata(structType);

    if (!metadata) {
        return AllocCodegenResult("Struct metadata not found: " + context.typeManager->getTypeName(baseType));
    }

    auto it = metadata->memberIndexMap.find(memberName);
    if (it == metadata->memberIndexMap.end()) {
        return AllocCodegenResult("Member not found in struct: " + memberName);
    }

    int memberIndex = it->second;
    llvm::Type* memberType = metadata->memberTypes[memberIndex];

    std::vector<llvm::Value *> indices;
    indices.push_back(context.builder.getInt32(0));
    indices.push_back(context.builder.getInt32(memberIndex));

    llvm::Value *memberPtr = context.builder.CreateGEP(
        baseType,
        baseValue,
        indices,
        "member_ptr");

    return AllocCodegenResult(memberPtr, memberType);
}

ExprCodegenResult NInteger::codegen(ASTContext &context) {
    return ExprCodegenResult(
        context.builder.getInt32(value),
        context.typeManager->getIntType()
    );
}

ExprCodegenResult NFloat::codegen(ASTContext &context) {
    return ExprCodegenResult(
        llvm::ConstantFP::get(llvm::Type::getDoubleTy(context.llvmContext), value),
        context.typeManager->getDoubleType()
    );
}

ExprCodegenResult NString::codegen(ASTContext &context) {
    return ExprCodegenResult(
        context.builder.CreateGlobalStringPtr(value, "string_literal"),
        context.typeManager->getPointerType(
            context.typeManager->getCharType())
    );
}

CodegenResult<llvm::Value*> NDeclarator::getArraySizeValue(ASTContext &context) {
    if (arrayDimensions.empty()) {
        return CodegenResult<llvm::Value*>("No array dimensions in declarator");
    }

    ExprCodegenResult sizeResult = arrayDimensions[0]->codegen(context);
    if (false == sizeResult.isSuccess()) {
        return CodegenResult<llvm::Value*>("Failed to generate code for array size expression") << sizeResult;
    }
    for (size_t i = 1; i < arrayDimensions.size(); ++i) {
        // For multi-dimensional arrays, we can multiply the sizes together
        sizeResult = NBinaryOperator(
            arrayDimensions[i - 1],
            BineryOperator::MUL,
            arrayDimensions[i]
        ).codegen(context);

        if (false == sizeResult.isSuccess()) {
            return CodegenResult<llvm::Value*>("Failed to compute total size for multi-dimensional array") << sizeResult;
        }
    }

    return CodegenResult<llvm::Value*>(sizeResult.getValue());
}

ExprCodegenResult NArraySubscript::codegen(ASTContext &context) {
    CodegenResult ptrResult = allocgen(context);
    if (false == ptrResult.isSuccess()) {
        return ExprCodegenResult("Failed to generate pointer for array subscript") << ptrResult;
    }

    llvm::Value *value = context.builder.CreateLoad(
        ptrResult.getType(),
        ptrResult.getAllocaInst(),
        "arrayelem"
    );

    return ExprCodegenResult(value, ptrResult.getType());
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
    llvm::Type* arrayType = arrayResult.getType();
    llvm::Type* elementType = nullptr;

    if (true == arrayType->isArrayTy()) {
        elementType = context.typeManager->getArrayElementType(arrayType);
        std::vector<llvm::Value*> indices(2);
        indices[0] = context.builder.getInt32(0);
        indices[1] = indexResult.getValue();

        llvm::Value *elementPtr = context.builder.CreateGEP(
            arrayType,
            basePtr,
            indices,
            "arrayidx"
        );

        return AllocCodegenResult(elementPtr, elementType);
    } else if (true == arrayType->isPointerTy()) {
        elementType = context.typeManager->getPointeeType(arrayType);

        // For pointer types, basePtr is an alloca storing the pointer
        // We need to load the pointer value first
        llvm::Value *ptrValue = context.builder.CreateLoad(
            arrayType,
            basePtr,
            "load_ptr"
        );

        llvm::Value *elementPtr = context.builder.CreateGEP(
            elementType,
            ptrValue,
            indexResult.getValue(),
            "ptridx"
        );

        return AllocCodegenResult(elementPtr, elementType);
    }

    std::string typeName = context.typeManager->getTypeName(arrayType);
    return AllocCodegenResult("Base is not an array type in subscript operation, got: " + typeName);
}

ExprCodegenResult NInitializerList::codegen(ASTContext &context) {
    return ExprCodegenResult("InitializerList cannot be used directly in expressions");
}

ExprCodegenResult NCastExpression::codegen(ASTContext &context) {
    // Generate code for the expression to be casted
    ExprCodegenResult exprResult = expr->codegen(context);
    if (!exprResult.isSuccess()) {
        return ExprCodegenResult("Failed to generate code for cast expression") << exprResult;
    }

    // Get the target type
    llvm::Type* targetType = context.typeManager->realize(targetTypeDesc, context);
    if (nullptr == targetType) {
        return ExprCodegenResult("Target type is null in cast expression");
    }

    // Get source and target types
    llvm::Type* sourceType = exprResult.getType();
    llvm::Value *sourceValue = exprResult.getValue();

    // Perform the type cast
    CodegenResult castResult = typeCast(sourceValue, sourceType, targetType, context);
    if (!castResult.isSuccess()) {
        return ExprCodegenResult("Type cast failed in cast expression") << castResult;
    }

    return ExprCodegenResult(castResult.getValue(), targetType);
}

ExprCodegenResult NSizeofExpression::codegen(ASTContext &context) {
    llvm::Type *type = nullptr;

    if (true == isSizeofType) {
        // sizeof(type)
        type = context.typeManager->realize(targetTypeDesc, context);
        if (nullptr == type) {
            return ExprCodegenResult("Target type is null in sizeof expression");
        }
    } else {
        // sizeof expression
        AllocCodegenResult allocResult = expr->allocgen(context);
        if (allocResult.isSuccess() && allocResult.getType()) {
            // Use the type from allocgen (preserves array type)
            type = allocResult.getType();
        } else {
            // Fallback to codegen for expressions that don't have lvalue
            ExprCodegenResult exprResult = expr->codegen(context);
            if (!exprResult.isSuccess()) {
                return ExprCodegenResult("Failed to generate code for sizeof expression") << exprResult;
            }

            type = exprResult.getType();
        }
    }

    // Get the size of the type
    llvm::DataLayout dataLayout(&(context.module));
    uint64_t sizeInBytes = dataLayout.getTypeAllocSize(type);

    // Return the size as a constant integer
    llvm::Value *sizeValue = llvm::ConstantInt::get(
        llvm::Type::getInt32Ty(context.llvmContext),
        sizeInBytes
    );

    return ExprCodegenResult(sizeValue, context.typeManager->getIntType());
}

ExprCodegenResult NCommaExpression::codegen(ASTContext &context) {
    // Evaluate left expression (for side effects)
    ExprCodegenResult leftResult = left->codegen(context);
    if (!leftResult.isSuccess()) {
        return ExprCodegenResult("Failed to generate code for left operand of comma expression") << leftResult;
    }

    // Evaluate and return right expression
    ExprCodegenResult rightResult = right->codegen(context);
    if (!rightResult.isSuccess()) {
        return ExprCodegenResult("Failed to generate code for right operand of comma expression") << rightResult;
    }

    // Comma operator evaluates left, then right, and returns right's value
    return rightResult;
}

