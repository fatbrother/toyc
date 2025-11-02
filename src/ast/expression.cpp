#include "ast/expression.hpp"
#include "ast/external_definition.hpp"
#include "ast/structure.hpp"

#include <iostream>
#include <map>

using namespace toyc::ast;

CodegenResult NBinaryOperator::codegen(ASTContext &context) {
    CodegenResult lhsResult = lhs->codegen(context);
    CodegenResult rhsResult = rhs->codegen(context);
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
        targetType = (lhsType->type > rhsType->type) ? lhsType->type : rhsType->type;
    } else if (op == AND || op == OR) {
        targetType = VAR_TYPE_BOOL;
        resultType = std::make_shared<NType>(VarType::VAR_TYPE_BOOL);
    } else {
        targetType = (true == isFloatingPointType(lhsType->type)) ? VAR_TYPE_DOUBLE : VAR_TYPE_INT;
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
        case AND:
            result = context.builder.CreateLogicalAnd(lhsValue, rhsValue, "and");
            break;
        case OR:
            result = context.builder.CreateLogicalOr(lhsValue, rhsValue, "or");
            break;
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
        resultType = type;
        resultType->pointerLevel++;
    } else if (op == DEREF) {
        resultType = type;
        resultType->pointerLevel--;
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

    context.builder.SetInsertPoint(trueBlock);
    CodegenResult trueResult = trueExpr->codegen(context);
    llvm::Value *trueValue = trueResult.getValue();
    NTypePtr trueType = trueResult.getType();
    if (false == trueResult.isSuccess() || nullptr == trueValue) {
        return trueResult << CodegenResult("True expression code generation failed");
    }
    context.builder.CreateBr(mergeBlock);

    context.builder.SetInsertPoint(falseBlock);
    CodegenResult falseResult = falseExpr->codegen(context);
    llvm::Value *falseValue = falseResult.getValue();
    NTypePtr falseType = falseResult.getType();
    if (false == falseResult.isSuccess() || nullptr == falseValue) {
        return falseResult << CodegenResult("False expression code generation failed");
    }
    context.builder.CreateBr(mergeBlock);

    context.builder.SetInsertPoint(mergeBlock);

    // Merge the two branches
    llvm::PHINode * phiNode = context.builder.CreatePHI(trueValue->getType(), 2, "phi");

    phiNode->addIncoming(trueValue, trueBlock);
    phiNode->addIncoming(falseValue, falseBlock);

    return CodegenResult(phiNode, trueType);
}

CodegenResult NIdentifier::codegen(ASTContext &context) {
    auto [isFound, variablePair] = context.variableTable->lookup(name);
    auto [allocaInst, type] = variablePair;
    llvm::Value *value = nullptr;
    if (false == isFound || nullptr == allocaInst) {
        return CodegenResult("Variable not found: " + name);
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
    for (paramIt = function->getParams(), argNode = argNodes; paramIt != nullptr && argNode != nullptr; paramIt = paramIt->next, argNode = argNodes->next) {
        CodegenResult argResult = argNode->codegen(context);
        llvm::Value *argValue = argResult.getValue();
        NTypePtr argType = argResult.getType();
        if (false == argResult.isSuccess() || nullptr == argValue) {
            return argResult << CodegenResult("Argument code generation failed for function call: " + name);
        }
        CodegenResult castResult = typeCast(
            argValue,
            argType,
            paramIt->getVarType()->type,
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

    if (baseType->pointerLevel > 0) {
        CodegenResult derefTypeResult = baseType->getLLVMType(context);
        if (false == derefTypeResult.isSuccess() || nullptr == derefTypeResult.getLLVMType()) {
            return derefTypeResult << CodegenResult("Failed to get LLVM type for dereferencing in member access");
        }
        baseValue = context.builder.CreateLoad(
            derefTypeResult.getLLVMType(),
            baseValue,
            "deref_member_access");
        baseType->pointerLevel--;
    }

    if (baseType->type != VarType::VAR_TYPE_STRUCT) {
        return CodegenResult("Base type is not a struct for member access: " + memberName);
    }

    auto [isFound, typePtr] = context.typeTable->lookup(baseType->name);
    if (false == isFound) {
        return CodegenResult("Struct type not found in type table: " + baseType->name);
    }
    NStructType *structType = static_cast<NStructType *>(typePtr.get());

    int memberIndex = structType->getMemberIndex(memberName);
    if (memberIndex < 0) {
        return CodegenResult("Member not found in struct: " + memberName);
    }

    std::vector<llvm::Value *> indices;
    indices.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.llvmContext), 0));
    indices.push_back(llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.llvmContext), memberIndex));

    llvm::Value *memberPtr = context.builder.CreateGEP(
        structType->llvmStructType,
        baseValue,
        indices,
        "member_ptr");

    NTypePtr memberType = structType->getMemberType(memberName);

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
        std::make_shared<NType>(VarType::VAR_TYPE_PTR, std::make_shared<NType>(VarType::VAR_TYPE_CHAR))
    );
}
