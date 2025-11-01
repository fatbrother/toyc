#include "ast/external_definition.hpp"

#include <iostream>
#include <llvm/IR/Verifier.h>

using namespace toyc::ast;

NParameter::NParameter(NType *type, NDeclarator *declarator)
    : isVariadic(false), type(nullptr), name("") {
    if (declarator) {
        name = declarator->getName();
        if (declarator->pointerLevel > 0) {
            this->type = std::make_shared<NType>(VarType::VAR_TYPE_PTR, type, declarator->pointerLevel);
        } else {
            this->type = std::shared_ptr<NType>(type);
        }
    }

    SAFE_DELETE(declarator);
}

NFunctionDefinition::~NFunctionDefinition() {
    SAFE_DELETE(params);
    SAFE_DELETE(body);
}

CodegenResult NFunctionDefinition::codegen(ASTContext &context) {
    llvm::Type *llvmReturnType = nullptr;
    std::vector<llvm::Type *> paramTypes;
    std::vector<std::string> paramNames;
    llvm::FunctionType *functionType = nullptr;
    bool isVariadic = false;

    CodegenResult returnTypeResult = returnType->getLLVMType(context);
    if (false == returnTypeResult.isSuccess() || nullptr == returnTypeResult.getLLVMType()) {
        return returnTypeResult << CodegenResult("Failed to get LLVM type for function return type");
    }
    llvmReturnType = returnTypeResult.getLLVMType();

    for (NParameter *paramIt = params; paramIt != nullptr; paramIt = paramIt->next) {
        if (true == paramIt->isVariadic) {
            isVariadic = true;
            break;
        }

        CodegenResult paramTypeResult = paramIt->getVarType()->getLLVMType(context);
        if (false == paramTypeResult.isSuccess() || nullptr == paramTypeResult.getLLVMType()) {
            return paramTypeResult << CodegenResult("Failed to get LLVM type for parameter");
        }
        llvm::Type *paramType = paramTypeResult.getLLVMType();
        if (nullptr == paramType) {
            return CodegenResult("Parameter type is null");
        }

        paramTypes.push_back(paramType);
        paramNames.push_back(paramIt->getName());
    }

    functionType = llvm::FunctionType::get(llvmReturnType, paramTypes, isVariadic);
    llvmFunction = static_cast<llvm::Function *>(context.module.getOrInsertFunction(name, functionType).getCallee());
    if (nullptr == llvmFunction) {
        return CodegenResult("Function creation failed for " + name);
    }

    NParameter* paramIt = params;
    for (auto it = llvmFunction->arg_begin(); it != llvmFunction->arg_end() && paramIt != nullptr; ++it) {
        it->setName(paramIt->getName());
        paramIt = paramIt->next;
    }

    context.functionDefinitions[name] = this;

    if (nullptr == body) {
        return CodegenResult(llvmFunction);
    }

    context.currentFunction = this;
    context.isInitializingFunction = true;
    body->setName("entry");
    CodegenResult bodyResult = body->codegen(context);
    if (false == bodyResult.isSuccess()) {
        return bodyResult << CodegenResult("Function body code generation failed for " + name);
    }
    context.currentFunction = nullptr;
    context.isInitializingFunction = false;

    std::string Error;
    llvm::raw_string_ostream ErrorOS(Error);
    if (false != llvm::verifyFunction(*llvmFunction, &ErrorOS)) {
        return CodegenResult(ErrorOS.str());
    }

    return CodegenResult(llvmFunction);
}