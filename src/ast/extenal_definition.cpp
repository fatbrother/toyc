#include "ast/external_definition.hpp"
#include "utility/raii_guard.hpp"

#include <iostream>
#include <llvm/IR/Verifier.h>

using namespace toyc::ast;

NParameter::NParameter(TypeDescriptor *typeDesc, NDeclarator *declarator)
    : isVariadic(false), typeDesc(typeDesc), name("") {
    if (declarator) {
        name = declarator->getName();

        if (declarator->isArray()) {
            this->typeDesc = new PointerTypeDescriptor(
                TypeDescriptorPtr(typeDesc),
                1
            );
        } else if (declarator->pointerLevel > 0) {
            this->typeDesc = new PointerTypeDescriptor(
                TypeDescriptorPtr(typeDesc),
                declarator->pointerLevel
            );
        }
    }

    SAFE_DELETE(declarator);
}

NParameter::~NParameter() {
    SAFE_DELETE(typeDesc);
    SAFE_DELETE(next);
}

NFunctionDefinition::~NFunctionDefinition() {
    SAFE_DELETE(returnTypeDesc);
    SAFE_DELETE(params);
    SAFE_DELETE(body);
}

StmtCodegenResult NFunctionDefinition::codegen(ASTContext &context) {
    returnType = context.typeFactory->realize(returnTypeDesc);
    if (!returnType) {
        return StmtCodegenResult("Failed to realize return type from descriptor");
    }

    llvm::Type *llvmReturnType = nullptr;
    std::vector<llvm::Type *> paramTypes;
    std::vector<std::string> paramNames;
    llvm::FunctionType *functionType = nullptr;
    bool isVariadic = false;

    TypeCodegenResult returnTypeResult = returnType->getLLVMType(context);
    if (false == returnTypeResult.isSuccess()) {
        return StmtCodegenResult("Failed to get LLVM type for function return type") << returnTypeResult;
    }
    llvmReturnType = returnTypeResult.getLLVMType();

    for (NParameter *paramIt = params; paramIt != nullptr; paramIt = paramIt->next) {
        if (true == paramIt->isVariadic) {
            isVariadic = true;
            break;
        }

        NTypePtr paramType = context.typeFactory->realize(paramIt->getTypeDescriptor());
        if (!paramType) {
            return StmtCodegenResult("Failed to realize parameter type from descriptor");
        }
        TypeCodegenResult paramTypeResult = paramType->getLLVMType(context);
        if (false == paramTypeResult.isSuccess()) {
            return StmtCodegenResult("Failed to get LLVM type for parameter") << paramTypeResult;
        }
        llvm::Type *paramLLVMType = paramTypeResult.getLLVMType();
        if (nullptr == paramLLVMType) {
            return StmtCodegenResult("Parameter type is null");
        }

        paramTypes.push_back(paramLLVMType);
        paramNames.push_back(paramIt->getName());
    }

    functionType = llvm::FunctionType::get(llvmReturnType, paramTypes, isVariadic);
    llvmFunction = static_cast<llvm::Function *>(context.module.getOrInsertFunction(name, functionType).getCallee());
    if (nullptr == llvmFunction) {
        return StmtCodegenResult("Function creation failed for " + name);
    }

    NParameter* paramIt = params;
    for (auto it = llvmFunction->arg_begin(); it != llvmFunction->arg_end() && paramIt != nullptr; ++it) {
        it->setName(paramIt->getName());
        paramIt = paramIt->next;
    }

    context.functionDefinitions[name] = this;

    if (nullptr == body) {
        return StmtCodegenResult();
    }

    context.currentFunction = this;
    context.isInitializingFunction = true;
    body->setName("entry");
    auto labelGuard = toyc::utility::makeScopeGuard([&context]() {
        context.clearLabels();
    });
    StmtCodegenResult bodyResult = body->codegen(context);
    if (false == bodyResult.isSuccess()) {
        return StmtCodegenResult("Function body code generation failed for " + name) << bodyResult;
    }
    context.currentFunction = nullptr;
    context.isInitializingFunction = false;

    // Resolve pending goto statements
    if (!context.pendingGotos.empty()) {
        return StmtCodegenResult("Undefined label in goto statement in function " + name);
    }

    std::string Error;
    llvm::raw_string_ostream ErrorOS(Error);
    if (false != llvm::verifyFunction(*llvmFunction, &ErrorOS)) {
        return StmtCodegenResult(ErrorOS.str());
    }

    return StmtCodegenResult();
}