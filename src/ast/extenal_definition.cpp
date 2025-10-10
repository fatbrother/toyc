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

llvm::Value *NFunctionDefinition::codegen(ASTContext &context) {
    llvm::Type *llvmReturnType = returnType->getLLVMType(context.llvmContext);
    std::vector<llvm::Type *> paramTypes;
    std::vector<std::string> paramNames;
    llvm::FunctionType *functionType = nullptr;
    bool isVariadic = false;

    for (NParameter *paramIt = params; paramIt != nullptr; paramIt = paramIt->next) {
        if (true == paramIt->isVariadic) {
            isVariadic = true;
            break;
        }

        llvm::Type *paramType = paramIt->getLLVMType(context.llvmContext);
        if (nullptr == paramType) {
            std::cerr << "Error: Parameter type is null" << std::endl;
            return nullptr;
        }

        paramTypes.push_back(paramType);
        paramNames.push_back(paramIt->getName());
    }

    functionType = llvm::FunctionType::get(llvmReturnType, paramTypes, isVariadic);
    llvmFunction = static_cast<llvm::Function *>(context.module.getOrInsertFunction(name, functionType).getCallee());
    if (nullptr == llvmFunction) {
        std::cerr << "Error: Function is null" << std::endl;
        return nullptr;
    }

    NParameter* paramIt = params;
    for (auto it = llvmFunction->arg_begin(); it != llvmFunction->arg_end() && paramIt != nullptr; ++it) {
        it->setName(paramIt->getName());
        paramIt = paramIt->next;
    }

    context.functionDefinitions[name] = this;

    if (nullptr == body) {
        return llvmFunction;
    }

    context.currentFunction = this;
    context.isInitializingFunction = true;
    body->setName("entry");
    if (nullptr == body->codegen(context)) {
        return nullptr;
    }
    context.currentFunction = nullptr;
    context.isInitializingFunction = false;

    if (false != llvm::verifyFunction(*llvmFunction, &llvm::errs())) {
        return nullptr;
    }

    return llvmFunction;
}