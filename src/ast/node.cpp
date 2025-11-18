#include "ast/node.hpp"

namespace toyc::ast {

ExprCodegenResult typeCast(llvm::Value *value, llvm::Type* fromType,
                      llvm::Type* toType, ASTContext &context) {
    if (!value || !fromType || !toType) {
        return ExprCodegenResult("Type cast failed due to null value or type");
    }

    if (fromType == toType) {
        return ExprCodegenResult(value, toType);
    }

    llvm::Value *result = nullptr;
    bool fromIsFloat = isFloatingPointType(fromType);
    bool toIsFloat = isFloatingPointType(toType);
    bool fromIsInt = isIntegerType(fromType);
    bool toIsInt = isIntegerType(toType);

    // Bool to X
    if (fromType->isIntegerTy(1)) {
        if (toIsInt) {
            result = context.builder.CreateZExt(value, toType, "bool_to_int");
        } else if (toIsFloat) {
            result = context.builder.CreateUIToFP(value, toType, "bool_to_float");
        } else {
            return ExprCodegenResult("Unsupported cast from bool to " + context.typeManager->getTypeName(toType));
        }
        return ExprCodegenResult(result, toType);
    }

    // X to Bool
    if (toType->isIntegerTy(1)) {
        if (fromIsFloat) {
            result = context.builder.CreateFCmpONE(value, llvm::ConstantFP::get(fromType, 0.0), "to_bool");
        } else if (fromIsInt) {
            result = context.builder.CreateICmpNE(value, llvm::ConstantInt::get(fromType, 0), "to_bool");
        } else {
            return ExprCodegenResult("Unsupported cast to bool from " + context.typeManager->getTypeName(fromType));
        }
        return ExprCodegenResult(result, toType);
    }

    // Int to Int
    if (fromIsInt && toIsInt) {
        result = context.builder.CreateIntCast(value, toType, true, "int_cast");
        return ExprCodegenResult(result, toType);
    }

    // Float to Int
    if (fromIsFloat && toIsInt) {
        result = context.builder.CreateFPToSI(value, toType, "float_to_int");
        return ExprCodegenResult(result, toType);
    }

    // Int to Float
    if (fromIsInt && toIsFloat) {
        result = context.builder.CreateSIToFP(value, toType, "int_to_float");
        return ExprCodegenResult(result, toType);
    }

    // Float to Float
    if (fromIsFloat && toIsFloat) {
        result = context.builder.CreateFPCast(value, toType, "float_cast");
        return ExprCodegenResult(result, toType);
    }

    // Pointer casts
    if (fromType->isPointerTy() && toType->isPointerTy()) {
        result = context.builder.CreateBitCast(value, toType, "ptr_cast");
        return ExprCodegenResult(result, toType);
    }

    // Int to Pointer
    if (fromIsInt && toType->isPointerTy()) {
        result = context.builder.CreateIntToPtr(value, toType, "int_to_ptr");
        return ExprCodegenResult(result, toType);
    }

    // Pointer to Int
    if (fromType->isPointerTy() && toIsInt) {
        result = context.builder.CreatePtrToInt(value, toType, "ptr_to_int");
        return ExprCodegenResult(result, toType);
    }

    return ExprCodegenResult("Unsupported type cast from " + context.typeManager->getTypeName(fromType) +
                           " to " + context.typeManager->getTypeName(toType));
}

ASTContext::ASTContext()
    : module("toyc", llvmContext),
      builder(llvmContext),
      typeManager(std::make_unique<TypeManager>(llvmContext, module)) {
    pushScope();
}

void ASTContext::pushJumpContext(std::shared_ptr<NJumpContext> ctx) {
    jumpContextStack.push(ctx);
}

void ASTContext::popJumpContext() {
    if (!jumpContextStack.empty()) {
        jumpContextStack.pop();
    }
}

std::shared_ptr<NJumpContext> ASTContext::getCurrentJumpContext() const {
    return jumpContextStack.empty() ? nullptr : jumpContextStack.top();
}

void ASTContext::registerLabel(const std::string& name, llvm::BasicBlock* block) {
    labels[name] = block;
}

llvm::BasicBlock* ASTContext::getLabel(const std::string& name) {
    auto it = labels.find(name);
    return (it != labels.end()) ? it->second : nullptr;
}

void ASTContext::clearLabels() {
    labels.clear();
    pendingGotos.clear();
}

void ASTContext::pushScope() {
    variableTable = new ScopeTable<std::pair<llvm::AllocaInst *, llvm::Type*>>(variableTable);
}

void ASTContext::popScope() {
    if (variableTable) {
        ScopeTable<std::pair<llvm::AllocaInst *, llvm::Type*>> *parent = variableTable->parent;
        delete variableTable;
        variableTable = parent;
    }
}


} // namespace toyc::ast
