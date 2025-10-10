#include "ast/expression.hpp"
#include "ast/statement.hpp"

#include <iostream>
#include <map>

using namespace toyc::ast;

std::pair<llvm::Value *, NTypePtr> NBinaryOperator::codegen(ASTContext &context) {
    auto [lhsValue, lhsType] = lhs->codegen(context);
    auto [rhsValue, rhsType] = rhs->codegen(context);
    llvm::Value *result = nullptr;
    NTypePtr resultType = nullptr;
    VarType targetType = VAR_TYPE_INT;

    if (nullptr == lhsValue || nullptr == rhsValue) {
        return std::make_pair(nullptr, nullptr);
    }

    if (op == EQ || op == NE || op == LE || op == GE || op == LT || op == GT) {
        resultType = std::make_shared<NType>(VarType::VAR_TYPE_BOOL);
        targetType = (lhsType->type > rhsType->type) ? lhsType->type : rhsType->type;
    } else if (op == AND || op == OR) {
        resultType = std::make_shared<NType>(VarType::VAR_TYPE_BOOL);
    } else {
        targetType = (true == isFloatingPointType(lhsType->type)) ? VAR_TYPE_DOUBLE : VAR_TYPE_INT;
        resultType = std::make_shared<NType>(targetType);
    }

    lhsValue = typeCast(lhsValue, lhsType, targetType, context.llvmContext, context.builder);
    rhsValue = typeCast(rhsValue, rhsType, targetType, context.llvmContext, context.builder);

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
            std::cerr << "Unknown binary operator: " << op << std::endl;
            return std::make_pair(nullptr, nullptr);
    }

    return std::make_pair(result, resultType);
}

std::pair<llvm::Value *, NTypePtr> NUnaryExpression::codegen(ASTContext &context) {
    auto [value, type] = expr->codegen(context);
    llvm::AllocaInst *allocaInst = nullptr;
    llvm::Value *one = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.llvmContext), 1);
    NTypePtr resultType = nullptr;

    if (nullptr == expr) {
        std::cerr << "Error: Expression is null" << std::endl;
        return std::make_pair(nullptr, nullptr);
    }

    if (op == L_INC || op == R_INC || op == L_DEC || op == R_DEC) {
        allocaInst = expr->allocgen(context).first;
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
        case DEREF:
            if (nullptr == value) {
                return std::make_pair(nullptr, nullptr);
            }
            value = context.builder.CreateLoad(type->getLLVMType(context.llvmContext), value, "deref");
            break;
        case PLUS:
            if (nullptr == value) {
                std::cerr << "Error: Unary plus failed" << std::endl;
                return std::make_pair(nullptr, nullptr);
            }
            break;
        case MINUS:
            if (nullptr == value) {
                std::cerr << "Error: Unary minus failed" << std::endl;
                return std::make_pair(nullptr, nullptr);
            }
            value = context.builder.CreateNeg(value, "neg");
            break;
        case BIT_NOT:
            if (nullptr == value) {
                std::cerr << "Error: Bitwise NOT failed" << std::endl;
                return std::make_pair(nullptr, nullptr);
            }
            value = context.builder.CreateXor(value, llvm::ConstantInt::get(value->getType(), -1), "bit_not");
            break;
        case LOG_NOT:
            value = typeCast(value, type, VarType::VAR_TYPE_BOOL, context.llvmContext, context.builder);
            value = context.builder.CreateICmpEQ(value, llvm::ConstantInt::get(value->getType(), 0), "log_not");
            break;
        default:
            std::cerr << "Unknown unary operator: " << op << std::endl;
            break;
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

    return std::make_pair(value, resultType);
}

std::pair<llvm::Value *, NTypePtr> NConditionalExpression::codegen(ASTContext &context) {
    if (nullptr == condition || nullptr == trueExpr || nullptr == falseExpr) {
        std::cerr << "Error: One of the expressions is null" << std::endl;
        return std::make_pair(nullptr, nullptr);
    }

    auto [condValue, condType] = condition->codegen(context);
    if (nullptr == condValue) {
        std::cerr << "Error: Condition code generation failed" << std::endl;
        return std::make_pair(nullptr, nullptr);
    }

    condValue = context.builder.CreateICmpNE(condValue, llvm::ConstantInt::get(condValue->getType(), 0), "cond");

    llvm::Function *function = context.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *trueBlock = llvm::BasicBlock::Create(context.llvmContext, "true", function);
    llvm::BasicBlock *falseBlock = llvm::BasicBlock::Create(context.llvmContext, "false", function);
    llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(context.llvmContext, "merge", function);

    context.builder.CreateCondBr(condValue, trueBlock, falseBlock);

    context.builder.SetInsertPoint(trueBlock);
    auto [trueValue, trueType] = trueExpr->codegen(context);
    if (nullptr == trueValue) {
        std::cerr << "Error: True expression code generation failed" << std::endl;
        return std::make_pair(nullptr, nullptr);
    }
    context.builder.CreateBr(mergeBlock);

    context.builder.SetInsertPoint(falseBlock);
    auto [falseValue, falseType] = falseExpr->codegen(context);
    if (nullptr == falseValue) {
        std::cerr << "Error: False expression code generation failed" << std::endl;
        return std::make_pair(nullptr, nullptr);
    }
    context.builder.CreateBr(mergeBlock);

    context.builder.SetInsertPoint(mergeBlock);

    // Merge the two branches
    llvm::PHINode * phiNode = context.builder.CreatePHI(trueValue->getType(), 2, "phi");

    phiNode->addIncoming(trueValue, trueBlock);
    phiNode->addIncoming(falseValue, falseBlock);

    return std::make_pair(phiNode, trueType);
}

std::pair<llvm::Value *, NTypePtr> NIdentifier::codegen(ASTContext &context) {
    auto [allocaInst, type] = context.variableTable->lookupVariable(name);
    llvm::Value *value = nullptr;
    if (nullptr == allocaInst) {
        std::cerr << "Error: Variable not found: " << name << std::endl;
        return std::make_pair(nullptr, nullptr);
    }

    value = context.builder.CreateLoad(allocaInst->getAllocatedType(), allocaInst, name);
    if (nullptr == value) {
        std::cerr << "Error: Load failed for variable: " << name << std::endl;
        return std::make_pair(nullptr, nullptr);
    }

    return std::make_pair(value, type);
}

std::pair<llvm::AllocaInst *, NTypePtr> NIdentifier::allocgen(ASTContext &context) {
    auto [allocaInst, type] = context.variableTable->lookupVariable(name);

    if (nullptr == allocaInst) {
        std::cerr << "Error: Variable not found: " << name << std::endl;
        return std::make_pair(nullptr, nullptr);
    }

    return std::make_pair(allocaInst, type);
}

std::pair<llvm::Value *, NTypePtr> NAssignment::codegen(ASTContext &context) {
    auto [lhsAlloca, lhsType] = lhs->allocgen(context);
    auto [rhsValue, rhsType] = rhs->codegen(context);
    if (nullptr == rhsValue || nullptr == lhsAlloca) {
        std::cerr << "Error: Assignment failed due to null values" << std::endl;
        return std::make_pair(nullptr, nullptr);
    }

    rhsValue = typeCast(rhsValue, rhsType, lhsType, context.llvmContext, context.builder);
    context.builder.CreateStore(rhsValue, lhsAlloca);
    return std::make_pair(rhsValue, lhsType);
}

std::pair<llvm::Value *, NTypePtr> NArguments::codegen(ASTContext &context) {
    return expr->codegen(context);
}

std::pair<llvm::Value *, NTypePtr> NFunctionCall::codegen(ASTContext &context) {
    std::vector<llvm::Value *> args;
    NFunctionDefinition *function = context.functionDefinitions[name];
    llvm::Value *res = nullptr;
    if (nullptr == function) {
        std::cerr << "Error: Function not found: " << name << std::endl;
        return std::make_pair(nullptr, nullptr);
    }

    NParameter *paramIt = nullptr;
    NArguments* argNode = nullptr;
    for (paramIt = function->getParams(), argNode = argNodes; paramIt != nullptr && argNode != nullptr; paramIt = paramIt->next, argNode = argNodes->next) {
        auto [argValue, argType] = argNode->codegen(context);
        argValue = typeCast(argValue, argType, paramIt->getVarType(), context.llvmContext, context.builder);
        if (nullptr == argValue) {
            std::cerr << "Error: Argument value is null" << std::endl;
            return std::make_pair(nullptr, nullptr);
        }
        args.push_back(argValue);
    }

    res = context.builder.CreateCall(function->getFunction(), args);

    return std::make_pair(res, function->getReturnType());
}

std::pair<llvm::Value *, NTypePtr> NInteger::codegen(ASTContext &context) {
    return std::make_pair(
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.llvmContext), value),
        std::make_shared<NType>(VarType::VAR_TYPE_INT)
    );
}

std::pair<llvm::Value *, NTypePtr> NFloat::codegen(ASTContext &context) {
    return std::make_pair(
        llvm::ConstantFP::get(llvm::Type::getDoubleTy(context.llvmContext), value),
        std::make_shared<NType>(VarType::VAR_TYPE_DOUBLE)
    );
}

std::pair<llvm::Value *, NTypePtr> NString::codegen(ASTContext &context) {
    return std::make_pair(
        context.builder.CreateGlobalStringPtr(value, "string_literal"),
        std::make_shared<NType>(VarType::VAR_TYPE_PTR, std::make_shared<NType>(VarType::VAR_TYPE_CHAR))
    );
}
