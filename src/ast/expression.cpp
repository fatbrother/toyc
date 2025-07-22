#include "ast/expression.hpp"
#include "ast/statement.hpp"

#include <iostream>
#include <map>

using namespace toyc::ast;

llvm::Value *NBinaryOperator::codegen(ASTContext &context) {
    llvm::Value *lhsValue = lhs->codegen(context);
    llvm::Value *rhsValue = rhs->codegen(context);
    llvm::Value *result = nullptr;

    if (nullptr == lhsValue || nullptr == rhsValue) {
        return nullptr;
    }

    std::map<BineryOperator, std::function<llvm::Value *(llvm::IRBuilder<> &, llvm::Value *, llvm::Value *)>> binaryOps = {
        {AND, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) {
            return typeCast(
                builder.CreateLogicalAnd(lhs, typeCast(rhs, VarType::VAR_TYPE_BOOL, builder.getContext(), builder), "and"), VarType::VAR_TYPE_INT, builder.getContext(), builder);
        }},
        {OR, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) {
            return typeCast(
                builder.CreateLogicalOr(lhs, typeCast(rhs, VarType::VAR_TYPE_BOOL, builder.getContext(), builder), "or"), VarType::VAR_TYPE_INT, builder.getContext(), builder);
        }},
        {EQ, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) {
            return typeCast(builder.CreateICmpEQ(lhs, rhs, "eq"), VarType::VAR_TYPE_INT, builder.getContext(), builder);
        }},
        {NE, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) {
            return typeCast(builder.CreateICmpNE(lhs, rhs, "ne"), VarType::VAR_TYPE_INT, builder.getContext(), builder);
        }},
        {LE, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) {
            return typeCast(builder.CreateICmpSLE(lhs, rhs, "le"), VarType::VAR_TYPE_INT, builder.getContext(), builder);
        }},
        {GE, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) {
            return typeCast(builder.CreateICmpSGE(lhs, rhs, "ge"), VarType::VAR_TYPE_INT, builder.getContext(), builder);
        }},
        {L, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) {
            return typeCast(builder.CreateICmpSLT(lhs, rhs, "lt"), VarType::VAR_TYPE_INT, builder.getContext(), builder);
        }},
        {G, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) {
            return typeCast(builder.CreateICmpSGT(lhs, rhs, "gt"), VarType::VAR_TYPE_INT, builder.getContext(), builder);
        }},
        {ADD, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateAdd(lhs, rhs, "add"); }},
        {SUB, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateSub(lhs, rhs, "sub"); }},
        {MUL, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateMul(lhs, rhs, "mul"); }},
        {DIV, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateSDiv(lhs, rhs, "div"); }},
        {MOD, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateSRem(lhs, rhs, "mod"); }},
        {LEFT, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateShl(lhs, rhs, "left"); }},
        {RIGHT, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateLShr(lhs, rhs, "right"); }},
        {BIT_AND, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateAnd(lhs, rhs, "bit_and"); }},
        {BIT_OR, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateOr(lhs, rhs, "bit_or"); }},
        {XOR, [](llvm::IRBuilder<> &builder, llvm::Value *lhs, llvm::Value *rhs) { return builder.CreateXor(lhs, rhs, "xor"); }}
    };

    if (0 != binaryOps.count(op)) {
        result = binaryOps[op](context.builder, lhsValue, rhsValue);
    } else {
        std::cerr << "Unknown binary operator: " << op << std::endl;
        return nullptr;
    }
    return result;
}

llvm::Value *NUnaryExpression::codegen(ASTContext &context) {
    llvm::Value *value = nullptr;
    llvm::Value *one = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.llvmContext), 1);

    if (nullptr == expr) {
        std::cerr << "Error: Expression is null" << std::endl;
        return nullptr;
    }

    llvm::Value *tmp = nullptr;
    switch (op) {
        case L_INC:
            value = expr->codegen(context);
            value = context.builder.CreateAdd(value, one, "inc");
            context.builder.CreateStore(value, expr->allocgen(context));
            break;
        case R_INC:
            value = expr->codegen(context);
            tmp = context.builder.CreateAdd(value, one, "inc");
            context.builder.CreateStore(tmp, expr->allocgen(context));
            break;
        case L_DEC:
            value = expr->codegen(context);
            value = context.builder.CreateSub(value, one, "dec");
            context.builder.CreateStore(value, expr->allocgen(context));
            break;
        case R_DEC:
            value = expr->codegen(context);
            tmp = context.builder.CreateSub(value, one, "dec");
            context.builder.CreateStore(tmp, expr->allocgen(context));
            break;
        case ADDR:
            value = expr->allocgen(context);
            break;
        case DEREF:
            value = expr->codegen(context);
            if (nullptr == value) {
                std::cerr << "Error: Dereference failed" << std::endl;
                return nullptr;
            }
            value = context.builder.CreateLoad(value->getType()->getPointerElementType(), value, "deref");
            break;
        case PLUS:
            value = expr->codegen(context);
            if (nullptr == value) {
                std::cerr << "Error: Unary plus failed" << std::endl;
                return nullptr;
            }
            break;
        case MINUS:
            value = expr->codegen(context);
            if (nullptr == value) {
                std::cerr << "Error: Unary minus failed" << std::endl;
                return nullptr;
            }
            value = context.builder.CreateNeg(value, "neg");
            break;
        case BIT_NOT:
            value = expr->codegen(context);
            if (nullptr == value) {
                std::cerr << "Error: Bitwise NOT failed" << std::endl;
                return nullptr;
            }
            value = context.builder.CreateXor(value, llvm::ConstantInt::get(value->getType(), -1), "bit_not");
            break;
        case LOG_NOT:
            value = typeCast(expr->codegen(context), VarType::VAR_TYPE_INT, context.llvmContext, context.builder);
            if (nullptr == value) {
                std::cerr << "Error: Logical NOT failed" << std::endl;
                return nullptr;
            }
            value = context.builder.CreateICmpEQ(value, llvm::ConstantInt::get(value->getType(), 0), "log_not");
            break;
        default:
            std::cerr << "Unknown unary operator: " << op << std::endl;
            break;
    }

    return value;
}

llvm::Value *NConditionalExpression::codegen(ASTContext &context) {
    if (nullptr == condition || nullptr == trueExpr || nullptr == falseExpr) {
        std::cerr << "Error: One of the expressions is null" << std::endl;
        return nullptr;
    }

    llvm::Value *condValue = condition->codegen(context);
    if (nullptr == condValue) {
        std::cerr << "Error: Condition code generation failed" << std::endl;
        return nullptr;
    }

    condValue = context.builder.CreateICmpNE(condValue, llvm::ConstantInt::get(condValue->getType(), 0), "cond");

    llvm::Function *function = context.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *trueBlock = llvm::BasicBlock::Create(context.llvmContext, "true", function);
    llvm::BasicBlock *falseBlock = llvm::BasicBlock::Create(context.llvmContext, "false", function);
    llvm::BasicBlock *mergeBlock = llvm::BasicBlock::Create(context.llvmContext, "merge", function);

    context.builder.CreateCondBr(condValue, trueBlock, falseBlock);

    context.builder.SetInsertPoint(trueBlock);
    llvm::Value *trueValue = trueExpr->codegen(context);
    if (nullptr == trueValue) {
        std::cerr << "Error: True expression code generation failed" << std::endl;
        return nullptr;
    }
    context.builder.CreateBr(mergeBlock);

    context.builder.SetInsertPoint(falseBlock);
    llvm::Value *falseValue = falseExpr->codegen(context);
    if (nullptr == falseValue) {
        std::cerr << "Error: False expression code generation failed" << std::endl;
        return nullptr;
    }
    context.builder.CreateBr(mergeBlock);

    context.builder.SetInsertPoint(mergeBlock);

    // Merge the two branches
    llvm::PHINode * phiNode = context.builder.CreatePHI(trueValue->getType(), 2, "phi");

    phiNode->addIncoming(trueValue, trueBlock);
    phiNode->addIncoming(falseValue, falseBlock);

    return phiNode;
}

llvm::Value *NIdentifier::codegen(ASTContext &context) {
    llvm::AllocaInst *allocaInst = context.variableTable->lookupVariable(name);
    llvm::Value *value = nullptr;
    if (nullptr == allocaInst) {
        std::cerr << "Error: Variable not found: " << name << std::endl;
        return nullptr;
    }

    value = context.builder.CreateLoad(allocaInst->getAllocatedType(), allocaInst, name);
    if (nullptr == value) {
        std::cerr << "Error: Load failed for variable: " << name << std::endl;
    }

    return value;
}

llvm::AllocaInst *NIdentifier::allocgen(ASTContext &context) {
    llvm::AllocaInst *allocaInst = context.variableTable->lookupVariable(name);
    if (nullptr == allocaInst) {
        std::cerr << "Error: Variable not found: " << name << std::endl;
        return nullptr;
    }

    return allocaInst;
}

llvm::Value *NAssignment::codegen(ASTContext &context) {
    llvm::AllocaInst *lhsAlloca = lhs->allocgen(context);
    llvm::Value *rhsValue = rhs->codegen(context);
    if (nullptr == rhsValue || nullptr == lhsAlloca) {
        std::cerr << "Error: Assignment failed due to null values" << std::endl;
        return nullptr;
    }

    if (lhsAlloca->getAllocatedType() != rhsValue->getType()) {
        std::cerr << "Error: Type mismatch in assignment" << std::endl;
        return nullptr;
    }
    context.builder.CreateStore(rhsValue, lhsAlloca);
    return rhsValue;
}

llvm::Value *NArguments::codegen(ASTContext &context) {
    if (nullptr == expr) {
        std::cerr << "Error: Expression is null" << std::endl;
        return nullptr;
    }
    return expr->codegen(context);
}

llvm::Value *NFunctionCall::codegen(ASTContext &context) {
    std::vector<llvm::Value *> args;
    llvm::Function *function = context.module.getFunction(name);
    llvm::Value *res = nullptr;
    if (nullptr == function) {
        std::cerr << "Error: Function not found: " << name << std::endl;
        return nullptr;
    }

    for (NArguments *argNode = argNodes; argNode != nullptr; argNode = argNode->next) {
        llvm::Value *argValue = argNode->codegen(context);
        if (nullptr == argValue) {
            std::cerr << "Error: Argument value is null" << std::endl;
            return nullptr;
        }
        args.push_back(argValue);
    }

    if (function->getReturnType()->isVoidTy()) {
        res = context.builder.CreateCall(function, args);
    } else {
        res = context.builder.CreateCall(function, args, "calltmp");
    }

    return res;
}

llvm::Value *NInteger::codegen(ASTContext &context) {
    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(context.llvmContext), value);
}

llvm::Value *NFloat::codegen(ASTContext &context) {
    return llvm::ConstantFP::get(llvm::Type::getDoubleTy(context.llvmContext), value);
}

llvm::Value *NString::codegen(ASTContext &context) {
    return context.builder.CreateGlobalStringPtr(value, "str");
}
