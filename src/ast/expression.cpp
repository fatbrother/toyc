#include "ast/expression.hpp"
#include "ast/external_definition.hpp"
#include "utility/type_cast.hpp"

#include <iostream>
#include <map>

using namespace toyc::ast;
using namespace toyc::utility;

ExprCodegenResult NLogicalOperator::codegen(ASTContext &context) {
    // Evaluate left operand
    ExprCodegenResult lhsResult = lhs->codegen(context);
    llvm::Value *lhsValue = lhsResult.getValue();
    NTypePtr lhsType = lhsResult.getType();

    if (false == lhsResult.isSuccess()) {
        return ExprCodegenResult("Failed to generate code for left operand in logical operation") << lhsResult;
    }

    // Cast lhs to bool
    CodegenResult castLhsResult = typeCast(lhsValue, lhsType, VAR_TYPE_BOOL, context);
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
    NTypePtr rhsType = rhsResult.getType();

    if (false == rhsResult.isSuccess()) {
        return ExprCodegenResult("Failed to generate code for right operand in logical operation") << rhsResult;
    }

    // Cast rhs to bool
    CodegenResult castRhsResult = typeCast(rhsValue, rhsType, VAR_TYPE_BOOL, context);
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

    return ExprCodegenResult(phiNode, context.typeFactory->getBasicType(VarType::VAR_TYPE_BOOL));
}

ExprCodegenResult NBinaryOperator::codegen(ASTContext &context) {
    CodegenResult rhsResult = rhs->codegen(context);
    ExprCodegenResult lhsResult = lhs->codegen(context);
    llvm::Value *result = nullptr;
    NTypePtr resultType = nullptr;
    VarType targetType = VAR_TYPE_INT;
    llvm::Value *lhsValue = lhsResult.getValue();
    NTypePtr lhsType = lhsResult.getType();
    llvm::Value *rhsValue = rhsResult.getValue();
    NTypePtr rhsType = rhsResult.getType();

    if (false == lhsResult.isSuccess() || false == rhsResult.isSuccess()) {
        return ExprCodegenResult("Failed to generate code for binary operator operands")
            << lhsResult << rhsResult;
    }

    targetType = (lhsType->getVarType() > rhsType->getVarType()) ? lhsType->getVarType() : rhsType->getVarType();
    if (op == EQ || op == NE || op == LE || op == GE || op == LT || op == GT) {
        resultType = context.typeFactory->getBasicType(VarType::VAR_TYPE_BOOL);
    } else {
        resultType = context.typeFactory->getBasicType(targetType);
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
            if (true == isFloatingPointType(targetType))
                result = context.builder.CreateFAdd(lhsValue, rhsValue, "add");
            else
                result = context.builder.CreateAdd(lhsValue, rhsValue, "add");
            break;
        case SUB:
            if (true == isFloatingPointType(targetType))
                result = context.builder.CreateFSub(lhsValue, rhsValue, "sub");
            else
                result = context.builder.CreateSub(lhsValue, rhsValue, "sub");
            break;
        case MUL:
            if (true == isFloatingPointType(targetType))
                result = context.builder.CreateFMul(lhsValue, rhsValue, "mul");
            else
                result = context.builder.CreateMul(lhsValue, rhsValue, "mul");
            break;
        case DIV:
            if (true == isFloatingPointType(targetType))
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
            result = (true == isFloatingPointType(targetType)) ?
                     context.builder.CreateFCmpOEQ(lhsValue, rhsValue, "eq") :
                     context.builder.CreateICmpEQ(lhsValue, rhsValue, "eq");
            break;
        case NE:
            result = (true == isFloatingPointType(targetType)) ?
                     context.builder.CreateFCmpONE(lhsValue, rhsValue, "ne") :
                     context.builder.CreateICmpNE(lhsValue, rhsValue, "ne");
            break;
        case LE:
            result = (true == isFloatingPointType(targetType)) ?
                     context.builder.CreateFCmpOLE(lhsValue, rhsValue, "le") :
                     context.builder.CreateICmpSLE(lhsValue, rhsValue, "le");
            break;
        case GE:
            result = (true == isFloatingPointType(targetType)) ?
                     context.builder.CreateFCmpOGE(lhsValue, rhsValue, "ge") :
                     context.builder.CreateICmpSGE(lhsValue, rhsValue, "ge");
            break;
        case LT:
            result = (true == isFloatingPointType(targetType)) ?
                     context.builder.CreateFCmpOLT(lhsValue, rhsValue, "lt") :
                     context.builder.CreateICmpSLT(lhsValue, rhsValue, "lt");
            break;
        case GT:
            result = (true == isFloatingPointType(targetType)) ?
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
    llvm::Value *one = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.llvmContext), 1);
    llvm::Value *value = exprResult.getValue();
    NTypePtr type = exprResult.getType();
    NTypePtr resultType = nullptr;

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
                NTypePtr pointeeType = type->getElementType(context);
                if (nullptr == pointeeType) {
                    return ExprCodegenResult("Cannot dereference non-pointer type");
                }

                CodegenResult derefTypeResult = pointeeType->getLLVMType(context);
                if (false == derefTypeResult.isSuccess()) {
                    return ExprCodegenResult("Failed to get LLVM type for dereference") << derefTypeResult;
                }
                value = context.builder.CreateLoad(derefTypeResult.getLLVMType(), value, "deref");
            }
            break;
        case PLUS:
            break;
        case MINUS:
            value = context.builder.CreateNeg(value, "neg");
            break;
        case BIT_NOT:
            value = context.builder.CreateXor(value, llvm::ConstantInt::get(value->getType(), -1), "bit_not");
            break;
        case LOG_NOT: {
                CodegenResult castResult = typeCast(value, type, VAR_TYPE_BOOL, context);
                if (false == castResult.isSuccess()) {
                    return ExprCodegenResult("Failed to cast value to bool for logical not") << castResult;
                }
                value = context.builder.CreateICmpEQ(value, llvm::ConstantInt::get(value->getType(), 0), "log_not");
            }
            break;
        default:
            return ExprCodegenResult("Unknown unary operator");
    }

    if (op == ADDR) {
        resultType = type->getAddrType(context);
    } else if (op == DEREF) {
        resultType = type->getElementType(context);
    } else if (op == LOG_NOT) {
        resultType = context.typeFactory->getBasicType(VarType::VAR_TYPE_BOOL);
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
    NTypePtr trueType = trueResult.getType();
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
    NTypePtr falseType = falseResult.getType();
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

    if (nullptr != type && true == type->isArray()) {
        std::vector<llvm::Value*> indices(2);
        indices[0] = llvm::ConstantInt::get(context.llvmContext, llvm::APInt(32, 0));
        indices[1] = llvm::ConstantInt::get(context.llvmContext, llvm::APInt(32, 0));

        value = context.builder.CreateGEP(
            allocaInst->getAllocatedType(),
            allocaInst,
            indices,
            name + "_decay"
        );

        return ExprCodegenResult(value, context.typeFactory->getPointerType(type->getElementType(context)));
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
    NTypePtr lhsType = lhsResult.getType();
    CodegenResult rhsResult = rhs->codegen(context);
    llvm::Value *rhsValue = rhsResult.getValue();
    NTypePtr rhsType = rhsResult.getType();
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

    NParameter *paramIt = nullptr;
    NArguments* argNode = nullptr;
    for (paramIt = function->getParams(), argNode = argNodes; paramIt != nullptr && argNode != nullptr; paramIt = paramIt->next, argNode = argNode->next) {

        CodegenResult argResult = argNode->codegen(context);
        llvm::Value *argValue = argResult.getValue();
        NTypePtr argType = argResult.getType();
        if (false == argResult.isSuccess()) {
            return ExprCodegenResult("Argument code generation failed for function call: " + name) << argResult;
        }

        if (false == paramIt->isVariadic) {
            CodegenResult castResult = typeCast(
                argValue,
                argType,
                context.typeFactory->realize(paramIt->getTypeDescriptor()),
                context);
            if (false == castResult.isSuccess()) {
                return ExprCodegenResult("Type cast failed for argument in function call: " + name) << castResult;
            }
            argValue = castResult.getValue();
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
    NTypePtr memberType = allocResult.getType();
    CodegenResult memberTypeResult = memberType->getLLVMType(context);
    if (false == memberTypeResult.isSuccess()) {
        return ExprCodegenResult("Failed to get LLVM type for member: " + memberName) << memberTypeResult;
    }
    llvm::Value *memberValue = context.builder.CreateLoad(
        memberTypeResult.getLLVMType(),
        memberPtr,
        "member_value");

    return ExprCodegenResult(memberValue, memberType);
}

AllocCodegenResult NMemberAccess::allocgen(ASTContext &context) {
    AllocCodegenResult baseResult = base->allocgen(context);
    llvm::Value *baseValue = baseResult.getAllocaInst();
    NTypePtr baseType = baseResult.getType();
    llvm::Type *childLLVMType = nullptr;

    if (false == baseResult.isSuccess()) {
        return AllocCodegenResult("Base expression code generation failed for member access: " + memberName) << baseResult;
    }

    if (true == baseType->isPointer() && true == isPointerAccess) {
        TypeCodegenResult derefTypeResult = baseType->getLLVMType(context);
        if (false == derefTypeResult.isSuccess() || nullptr == derefTypeResult.getLLVMType()) {
            return AllocCodegenResult("Failed to get LLVM type for dereferencing in member access") << derefTypeResult;
        }
        baseValue = context.builder.CreateLoad(
            derefTypeResult.getLLVMType(),
            baseValue,
            "deref_base");
        baseType = baseType->getElementType(context);
    }

    if (false == baseType->isStruct()) {
        return AllocCodegenResult("Base type is not a struct for member access: " + memberName);
    }

    auto [isFound, typeDef] = context.typeTable->lookup(baseType->getName());
    if (false == isFound || nullptr == typeDef) {
        return AllocCodegenResult("Struct type not found: " + baseType->getName());
    }
    auto structDef = std::static_pointer_cast<NStructType>(typeDef);
    int memberIndex = structDef->getMemberIndex(memberName);
    if (memberIndex < 0) {
        return AllocCodegenResult("Member not found in struct: " + memberName);
    }

    std::vector<llvm::Value *> indices;
    indices.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.llvmContext), 0));
    indices.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.llvmContext), memberIndex));

    llvm::Value *memberPtr = context.builder.CreateGEP(
        structDef->getLLVMType(context).getLLVMType(),
        baseValue,
        indices,
        "member_ptr");

    NTypePtr memberType = structDef->getMemberType(memberName, context);

    return AllocCodegenResult(memberPtr, memberType);
}

ExprCodegenResult NInteger::codegen(ASTContext &context) {
    return ExprCodegenResult(
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.llvmContext), value),
        context.typeFactory->getBasicType(VarType::VAR_TYPE_INT)
    );
}

ExprCodegenResult NFloat::codegen(ASTContext &context) {
    return ExprCodegenResult(
        llvm::ConstantFP::get(llvm::Type::getDoubleTy(context.llvmContext), value),
        context.typeFactory->getBasicType(VarType::VAR_TYPE_DOUBLE)
    );
}

ExprCodegenResult NString::codegen(ASTContext &context) {
    return ExprCodegenResult(
        context.builder.CreateGlobalStringPtr(value, "string_literal"),
        context.typeFactory->getPointerType(
            context.typeFactory->getCharType())
    );
}

ExprCodegenResult NArraySubscript::codegen(ASTContext &context) {
    CodegenResult ptrResult = allocgen(context);
    if (false == ptrResult.isSuccess()) {
        return ExprCodegenResult("Failed to generate pointer for array subscript") << ptrResult;
    }

    llvm::Value *value = context.builder.CreateLoad(
        ptrResult.getType()->getLLVMType(context).getLLVMType(),
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

    CodegenResult arrayCodegenResult = array->codegen(context);
    if (arrayCodegenResult.isSuccess() && arrayCodegenResult.getType() && arrayCodegenResult.getType()->isPointer()) {
        CodegenResult elemLLVMTypeResult = arrayCodegenResult.getType()->getElementType(context)->getLLVMType(context);
        if (false == elemLLVMTypeResult.isSuccess()) {
            return AllocCodegenResult("Failed to get element type for pointer subscript") << elemLLVMTypeResult;
        }

        llvm::Value *elementPtr = context.builder.CreateGEP(
            elemLLVMTypeResult.getLLVMType(),
            arrayCodegenResult.getValue(),
            indexResult.getValue(),
            "arrayidx"
        );

        return AllocCodegenResult(elementPtr, arrayCodegenResult.getType()->getElementType(context));
    }

    CodegenResult arrayResult = array->allocgen(context);
    if (false == arrayResult.isSuccess()) {
        return AllocCodegenResult("Failed to generate lvalue for array in subscript operation") << arrayResult;
    }

    llvm::Value *basePtr = arrayResult.getAllocaInst();
    NTypePtr elemType = arrayResult.getType()->getElementType(context);
    if (nullptr == elemType) {
        return AllocCodegenResult("Array type does not have an element type in subscript operation");
    }
    std::vector<llvm::Value*> indices(2);
    indices[0] = llvm::ConstantInt::get(context.llvmContext, llvm::APInt(32, 0));
    indices[1] = indexResult.getValue();

    CodegenResult typeResult = elemType->getLLVMType(context);
    if (false == typeResult.isSuccess()) {
        return AllocCodegenResult("Failed to get LLVM type for array element in subscript operation") << typeResult;
    }

    llvm::Value *elementPtr = context.builder.CreateGEP(
        arrayResult.getType()->getLLVMType(context).getLLVMType(),
        basePtr,
        indices,
        "arrayidx"
    );

    return AllocCodegenResult(elementPtr, elemType);
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
    NTypePtr targetType = context.typeFactory->realize(targetTypeDesc);
    if (nullptr == targetType) {
        return ExprCodegenResult("Target type is null in cast expression");
    }

    CodegenResult targetTypeLLVMResult = targetType->getLLVMType(context);
    if (!targetTypeLLVMResult.isSuccess()) {
        return ExprCodegenResult("Failed to get LLVM type for cast target") << targetTypeLLVMResult;
    }

    // Get source and target types
    NTypePtr sourceType = exprResult.getType();
    llvm::Value *sourceValue = exprResult.getValue();
    llvm::Type *targetLLVMType = targetTypeLLVMResult.getLLVMType();

    // Perform the type cast
    CodegenResult castResult = typeCast(sourceValue, sourceType, targetType, context);
    if (!castResult.isSuccess()) {
        return ExprCodegenResult("Type cast failed in cast expression") << castResult;
    }

    return ExprCodegenResult(castResult.getValue(), targetType);
}

ExprCodegenResult NSizeofExpression::codegen(ASTContext &context) {
    llvm::Type *llvmType = nullptr;

    if (isSizeofType) {
        // sizeof(type)
        NTypePtr targetType = context.typeFactory->realize(targetTypeDesc);
        if (nullptr == targetType) {
            return ExprCodegenResult("Target type is null in sizeof expression");
        }

        CodegenResult typeResult = targetType->getLLVMType(context);
        if (!typeResult.isSuccess()) {
            return ExprCodegenResult("Failed to get LLVM type for sizeof operand") << typeResult;
        }
        llvmType = typeResult.getLLVMType();
    } else {
        // sizeof expression
        AllocCodegenResult allocResult = expr->allocgen(context);
        if (allocResult.isSuccess() && allocResult.getType()) {
            // Use the type from allocgen (preserves array type)
            CodegenResult typeResult = allocResult.getType()->getLLVMType(context);
            if (!typeResult.isSuccess()) {
                return ExprCodegenResult("Failed to get LLVM type from expression") << typeResult;
            }
            llvmType = typeResult.getLLVMType();
        } else {
            // Fallback to codegen for expressions that don't have lvalue
            ExprCodegenResult exprResult = expr->codegen(context);
            if (!exprResult.isSuccess()) {
                return ExprCodegenResult("Failed to generate code for sizeof expression") << exprResult;
            }

            TypeCodegenResult typeResult = exprResult.getType()->getLLVMType(context);
            if (!typeResult.isSuccess()) {
                return ExprCodegenResult("Failed to get LLVM type from expression") << typeResult;
            }
            llvmType = typeResult.getLLVMType();
        }
    }

    // Get the size of the type
    llvm::DataLayout dataLayout(&(context.module));
    uint64_t sizeInBytes = dataLayout.getTypeAllocSize(llvmType);

    // Return the size as a constant integer
    llvm::Value *sizeValue = llvm::ConstantInt::get(
        llvm::Type::getInt32Ty(context.llvmContext),
        sizeInBytes
    );

    return ExprCodegenResult(sizeValue, context.typeFactory->getIntType());
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

