#include "ast/expression.hpp"
#include "ast/external_definition.hpp"
#include "utility/type_cast.hpp"

#include <iostream>
#include <map>

using namespace toyc::ast;
using namespace toyc::utility;

CodegenResult NLogicalOperator::codegen(ASTContext &context) {
    // Evaluate left operand
    CodegenResult lhsResult = lhs->codegen(context);
    llvm::Value *lhsValue = lhsResult.getValue();
    NTypePtr lhsType = lhsResult.getType();

    if (false == lhsResult.isSuccess() || nullptr == lhsValue) {
        return lhsResult << CodegenResult("Failed to generate code for left operand in logical operation");
    }

    // Cast lhs to bool
    CodegenResult castLhsResult = typeCast(lhsValue, lhsType, VAR_TYPE_BOOL, context);
    if (false == castLhsResult.isSuccess() || nullptr == castLhsResult.getValue()) {
        return castLhsResult << CodegenResult("Type cast failed for left-hand side in logical operation");
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

    if (false == rhsResult.isSuccess() || nullptr == rhsValue) {
        return rhsResult << CodegenResult("Failed to generate code for right operand in logical operation");
    }

    // Cast rhs to bool
    CodegenResult castRhsResult = typeCast(rhsValue, rhsType, VAR_TYPE_BOOL, context);
    if (false == castRhsResult.isSuccess() || nullptr == castRhsResult.getValue()) {
        return castRhsResult << CodegenResult("Type cast failed for right-hand side in logical operation");
    }
    rhsValue = castRhsResult.getValue();

    llvm::BasicBlock *rhsEndBlock = context.builder.GetInsertBlock();
    context.builder.CreateBr(mergeBlock);

    // Merge block with PHI node
    context.builder.SetInsertPoint(mergeBlock);
    llvm::PHINode *phiNode = context.builder.CreatePHI(lhsValue->getType(), 2, "logical_result");
    phiNode->addIncoming(lhsValue, lhsEndBlock);
    phiNode->addIncoming(rhsValue, rhsEndBlock);

    return CodegenResult(phiNode, std::make_shared<NType>(VarType::VAR_TYPE_BOOL));
}

CodegenResult NBinaryOperator::codegen(ASTContext &context) {
    CodegenResult rhsResult = rhs->codegen(context);
    CodegenResult lhsResult = lhs->codegen(context);
    llvm::Value *result = nullptr;
    NTypePtr resultType = nullptr;
    VarType targetType = VAR_TYPE_INT;
    llvm::Value *lhsValue = lhsResult.getValue();
    NTypePtr lhsType = lhsResult.getType();
    llvm::Value *rhsValue = rhsResult.getValue();
    NTypePtr rhsType = rhsResult.getType();

    if (false == lhsResult.isSuccess() || false == rhsResult.isSuccess()) {
        return lhsResult << rhsResult << CodegenResult("Failed to generate code for binary operator operands");
    }

    if (op == EQ || op == NE || op == LE || op == GE || op == LT || op == GT) {
        resultType = std::make_shared<NType>(VarType::VAR_TYPE_BOOL);
        targetType = (lhsType->getVarType() > rhsType->getVarType()) ? lhsType->getVarType() : rhsType->getVarType();
    } else {
        targetType = (true == isFloatingPointType(lhsType->getVarType())) ? VAR_TYPE_DOUBLE : VAR_TYPE_INT;
        resultType = std::make_shared<NType>(targetType);
    }

    CodegenResult castLhsResult = typeCast(
        lhsValue,
        lhsType,
        targetType,
        context);
    if (false == castLhsResult.isSuccess() || nullptr == castLhsResult.getValue()) {
        return castLhsResult << CodegenResult("Type cast failed for left-hand side in binary operation");
    }
    lhsValue = castLhsResult.getValue();

    CodegenResult castRhsResult = typeCast(
        rhsValue,
        rhsType,
        targetType,
        context);
    if (false == castRhsResult.isSuccess() || nullptr == castRhsResult.getValue()) {
        return castRhsResult << CodegenResult("Type cast failed for right-hand side in binary operation");
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
            return CodegenResult("Unknown binary operator");
    }

    return CodegenResult(result, resultType);
}

CodegenResult NUnaryExpression::codegen(ASTContext &context) {
    CodegenResult exprResult = expr->codegen(context);
    llvm::AllocaInst *allocaInst = nullptr;
    llvm::Value *one = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.llvmContext), 1);
    llvm::Value *value = exprResult.getValue();
    NTypePtr type = exprResult.getType();
    NTypePtr resultType = nullptr;

    if (false == exprResult.isSuccess() || nullptr == value) {
        return exprResult << CodegenResult("Failed to generate code for unary expression operand");
    }

    if (op == L_INC || op == R_INC || op == L_DEC || op == R_DEC) {
        CodegenResult allocResult = expr->allocgen(context);
        allocaInst = static_cast<llvm::AllocaInst *>(allocResult.getValue());
        if (false == allocResult.isSuccess() || nullptr == allocaInst) {
            return allocResult << CodegenResult("Failed to get lvalue for unary increment/decrement");
        }
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
                CodegenResult derefTypeResult = type->getLLVMType(context);
                if (false == derefTypeResult.isSuccess() || nullptr == derefTypeResult.getLLVMType()) {
                    return derefTypeResult << CodegenResult("Failed to get LLVM type for dereference");
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
                if (false == castResult.isSuccess() || nullptr == castResult.getValue()) {
                    return castResult << CodegenResult("Failed to cast value to bool for logical not");
                }
                value = context.builder.CreateICmpEQ(value, llvm::ConstantInt::get(value->getType(), 0), "log_not");
            }
            break;
        default:
            return CodegenResult("Unknown unary operator");
    }

    if (op == ADDR) {
        resultType = type->getAddrType();
    } else if (op == DEREF) {
        resultType = type->getElementType();
    } else if (op == LOG_NOT) {
        resultType = std::make_shared<NType>(VarType::VAR_TYPE_BOOL);
    } else {
        resultType = type;
    }

    return CodegenResult(value, resultType);
}

CodegenResult NConditionalExpression::codegen(ASTContext &context) {
    if (nullptr == condition || nullptr == trueExpr || nullptr == falseExpr) {
        return CodegenResult("One of the expressions is null");
    }

    CodegenResult condResult = condition->codegen(context);
    llvm::Value *condValue = condResult.getValue();
    if (false == condResult.isSuccess() || nullptr == condValue) {
        return condResult << CodegenResult("Condition expression code generation failed");
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
    if (false == trueResult.isSuccess() || nullptr == trueValue) {
        return trueResult << CodegenResult("True expression code generation failed");
    }
    // Get the actual block after codegen (it might have changed due to nested expressions)
    llvm::BasicBlock *trueEndBlock = context.builder.GetInsertBlock();
    context.builder.CreateBr(mergeBlock);

    // Generate false branch
    context.builder.SetInsertPoint(falseBlock);
    CodegenResult falseResult = falseExpr->codegen(context);
    llvm::Value *falseValue = falseResult.getValue();
    NTypePtr falseType = falseResult.getType();
    if (false == falseResult.isSuccess() || nullptr == falseValue) {
        return falseResult << CodegenResult("False expression code generation failed");
    }
    // Get the actual block after codegen (it might have changed due to nested expressions)
    llvm::BasicBlock *falseEndBlock = context.builder.GetInsertBlock();
    context.builder.CreateBr(mergeBlock);

    context.builder.SetInsertPoint(mergeBlock);

    // Merge the two branches using the actual ending blocks
    llvm::PHINode * phiNode = context.builder.CreatePHI(trueValue->getType(), 2, "phi");

    phiNode->addIncoming(trueValue, trueEndBlock);
    phiNode->addIncoming(falseValue, falseEndBlock);

    return CodegenResult(phiNode, trueType);
}

CodegenResult NIdentifier::codegen(ASTContext &context) {
    auto [isFound, variablePair] = context.variableTable->lookup(name);
    auto [allocaInst, type] = variablePair;
    llvm::Value *value = nullptr;
    if (false == isFound || nullptr == allocaInst) {
        return CodegenResult("Variable not found: " + name);
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

        return CodegenResult(value, std::make_shared<NPointerType>(type->getElementType()));
    }

    value = context.builder.CreateLoad(allocaInst->getAllocatedType(), allocaInst, name);
    if (nullptr == value) {
        return CodegenResult("Load failed for variable: " + name);
    }

    return CodegenResult(value, type);
}

CodegenResult NIdentifier::allocgen(ASTContext &context) {
    auto [isFound, variablePair] = context.variableTable->lookup(name);
    auto [allocaInst, type] = variablePair;

    if (false == isFound || nullptr == allocaInst) {
        return CodegenResult("Variable not found: " + name);
    }

    return CodegenResult(allocaInst, type);
}

CodegenResult NAssignment::codegen(ASTContext &context) {
    CodegenResult lhsResult = lhs->allocgen(context);
    llvm::AllocaInst *lhsAlloca = static_cast<llvm::AllocaInst *>(lhsResult.getValue());
    NTypePtr lhsType = lhsResult.getType();
    CodegenResult rhsResult = rhs->codegen(context);
    llvm::Value *rhsValue = rhsResult.getValue();
    NTypePtr rhsType = rhsResult.getType();
    if (false == rhsResult.isSuccess() || false == lhsResult.isSuccess() || nullptr == lhsAlloca || nullptr == rhsValue) {
        return lhsResult << rhsResult << CodegenResult("Assignment failed due to null values");
    }

    CodegenResult castResult = typeCast(
        rhsValue,
        rhsType,
        lhsType,
        context);
    if (false == castResult.isSuccess() || nullptr == castResult.getValue()) {
        return castResult << CodegenResult("Type cast failed during assignment");
    }
    rhsValue = castResult.getValue();
    context.builder.CreateStore(rhsValue, lhsAlloca);
    return CodegenResult(rhsValue, lhsType);
}

CodegenResult NArguments::codegen(ASTContext &context) {
    return expr->codegen(context);
}

CodegenResult NFunctionCall::codegen(ASTContext &context) {
    std::vector<llvm::Value *> args;
    NFunctionDefinition *function = context.functionDefinitions[name];
    llvm::Value *res = nullptr;
    if (nullptr == function) {
        return CodegenResult("Function not found: " + name);
    }

    NParameter *paramIt = nullptr;
    NArguments* argNode = nullptr;
    for (paramIt = function->getParams(), argNode = argNodes; paramIt != nullptr && argNode != nullptr; paramIt = paramIt->next, argNode = argNode->next) {
        CodegenResult argResult = argNode->codegen(context);
        llvm::Value *argValue = argResult.getValue();
        NTypePtr argType = argResult.getType();
        if (false == argResult.isSuccess() || nullptr == argValue) {
            return argResult << CodegenResult("Argument code generation failed for function call: " + name);
        }
        CodegenResult castResult = typeCast(
            argValue,
            argType,
            paramIt->getVarType(),
            context);
        if (false == castResult.isSuccess() || nullptr == castResult.getValue()) {
            return castResult << CodegenResult("Type cast failed for argument in function call: " + name);
        }
        args.push_back(castResult.getValue());
    }

    res = context.builder.CreateCall(function->getFunction(), args);

    return CodegenResult(res, function->getReturnType());
}

CodegenResult NMemberAccess::codegen(ASTContext &context) {
    CodegenResult allocResult = allocgen(context);
    if (false == allocResult.isSuccess()) {
        return allocResult << CodegenResult("Member access allocation failed for member: " + memberName);
    }

    llvm::Value *memberPtr = allocResult.getValue();
    NTypePtr memberType = allocResult.getType();
    CodegenResult memberTypeResult = memberType->getLLVMType(context);
    if (false == memberTypeResult.isSuccess() || nullptr == memberTypeResult.getLLVMType()) {
        return memberTypeResult << CodegenResult("Failed to get LLVM type for member: " + memberName);
    }
    llvm::Value *memberValue = context.builder.CreateLoad(
        memberTypeResult.getLLVMType(),
        memberPtr,
        "member_value");

    return CodegenResult(memberValue, memberType);
}

CodegenResult NMemberAccess::allocgen(ASTContext &context) {
    CodegenResult baseResult = base->allocgen(context);
    llvm::Value *baseValue = baseResult.getValue();
    NTypePtr baseType = baseResult.getType();
    llvm::Type *childLLVMType = nullptr;

    if (false == baseResult.isSuccess() || nullptr == baseValue) {
        return baseResult << CodegenResult("Base expression code generation failed for member access: " + memberName);
    }

    if (true == baseType->isPointer() && true == isPointerAccess) {
        CodegenResult derefTypeResult = baseType->getLLVMType(context);
        if (false == derefTypeResult.isSuccess() || nullptr == derefTypeResult.getLLVMType()) {
            return derefTypeResult << CodegenResult("Failed to get LLVM type for dereferencing in member access");
        }
        baseValue = context.builder.CreateLoad(
            derefTypeResult.getLLVMType(),
            baseValue,
            "deref_base");
        baseType = baseType->getElementType();
    }

    if (false == baseType->isStruct()) {
        return CodegenResult("Base type is not a struct for member access: " + memberName);
    }

    auto [isFound, typeDef] = context.typeTable->lookup(baseType->getName());
    if (false == isFound || nullptr == typeDef) {
        return CodegenResult("Struct type not found: " + baseType->getName());
    }
    auto structDef = std::static_pointer_cast<NStructType>(typeDef);
    int memberIndex = structDef->getMemberIndex(memberName);
    if (memberIndex < 0) {
        return CodegenResult("Member not found in struct: " + memberName);
    }

    std::vector<llvm::Value *> indices;
    indices.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.llvmContext), 0));
    indices.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.llvmContext), memberIndex));

    llvm::Value *memberPtr = context.builder.CreateGEP(
        structDef->getLLVMType(context).getLLVMType(),
        baseValue,
        indices,
        "member_ptr");

    NTypePtr memberType = structDef->getMemberType(memberName);

    return CodegenResult(memberPtr, memberType);
}

CodegenResult NInteger::codegen(ASTContext &context) {
    return CodegenResult(
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.llvmContext), value),
        std::make_shared<NType>(VarType::VAR_TYPE_INT)
    );
}

CodegenResult NFloat::codegen(ASTContext &context) {
    return CodegenResult(
        llvm::ConstantFP::get(llvm::Type::getDoubleTy(context.llvmContext), value),
        std::make_shared<NType>(VarType::VAR_TYPE_DOUBLE)
    );
}

CodegenResult NString::codegen(ASTContext &context) {
    return CodegenResult(
        context.builder.CreateGlobalStringPtr(value, "string_literal"),
        std::make_shared<NPointerType>(VarType::VAR_TYPE_CHAR)
    );
}

CodegenResult NArraySubscript::codegen(ASTContext &context) {
    CodegenResult ptrResult = allocgen(context);
    if (false == ptrResult.isSuccess()) {
        return ptrResult << CodegenResult("Failed to generate pointer for array subscript");
    }

    llvm::Value *value = context.builder.CreateLoad(
        ptrResult.getType()->getLLVMType(context).getLLVMType(),
        ptrResult.getValue(),
        "arrayelem"
    );

    return CodegenResult(value, ptrResult.getType());
}

CodegenResult NArraySubscript::allocgen(ASTContext &context) {
    CodegenResult indexResult = index->codegen(context);
    if (false == indexResult.isSuccess()) {
        return indexResult << CodegenResult("Failed to generate code for array index expression");
    }

    CodegenResult arrayCodegenResult = array->codegen(context);
    if (arrayCodegenResult.isSuccess() && arrayCodegenResult.getType() && arrayCodegenResult.getType()->isPointer()) {
        CodegenResult elemLLVMTypeResult = arrayCodegenResult.getType()->getElementType()->getLLVMType(context);
        if (false == elemLLVMTypeResult.isSuccess()) {
            return elemLLVMTypeResult << CodegenResult("Failed to get element type for pointer subscript");
        }

        llvm::Value *elementPtr = context.builder.CreateGEP(
            elemLLVMTypeResult.getLLVMType(),
            arrayCodegenResult.getValue(),
            indexResult.getValue(),
            "arrayidx"
        );

        return CodegenResult(elementPtr, arrayCodegenResult.getType()->getElementType());
    }

    CodegenResult arrayResult = array->allocgen(context);
    if (false == arrayResult.isSuccess()) {
        return arrayResult << CodegenResult("Failed to generate lvalue for array in subscript operation");
    }

    llvm::Value *basePtr = arrayResult.getValue();
    NTypePtr elemType = arrayResult.getType()->getElementType();
    std::vector<llvm::Value*> indices(2);
    indices[0] = llvm::ConstantInt::get(context.llvmContext, llvm::APInt(32, 0));
    indices[1] = indexResult.getValue();

    CodegenResult typeResult = elemType->getLLVMType(context);
    if (false == typeResult.isSuccess() || nullptr == typeResult.getLLVMType()) {
        return typeResult << CodegenResult("Failed to get LLVM type for array element in subscript operation");
    }

    llvm::Value *elementPtr = context.builder.CreateGEP(
        arrayResult.getType()->getLLVMType(context).getLLVMType(),
        basePtr,
        indices,
        "arrayidx"
    );

    return CodegenResult(elementPtr, elemType);
}

CodegenResult NInitializerList::codegen(ASTContext &context) {
    return CodegenResult("InitializerList cannot be used directly in expressions");
}

