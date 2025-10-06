#include "ast/statement.hpp"

#include <iostream>
#include <llvm/IR/Verifier.h>

using namespace toyc::ast;

NFunctionDefinition::~NFunctionDefinition() {
    SAFE_DELETE(params);
    SAFE_DELETE(body);
}

int NFunctionDefinition::codegen(ASTContext &context) {
    llvm::Type *llvmReturnType = returnType->getLLVMType(context.llvmContext);
    std::vector<llvm::Type *> paramTypes;
    std::vector<std::string> paramNames;
    llvm::FunctionType *functionType = nullptr;
    bool isVarArg = false;

    for (NParameter *paramIt = params; paramIt != nullptr; paramIt = paramIt->next) {
        if (true == paramIt->isVarArg) {
            isVarArg = true;
            break;
        }

        llvm::Type *paramType = paramIt->getLLVMType(context.llvmContext);
        if (nullptr == paramType) {
            std::cerr << "Error: Parameter type is null" << std::endl;
            return -1;
        }

        paramTypes.push_back(paramType);
        paramNames.push_back(paramIt->getName());
    }

    functionType = llvm::FunctionType::get(llvmReturnType, paramTypes, isVarArg);
    llvmFunction = static_cast<llvm::Function *>(context.module.getOrInsertFunction(name, functionType).getCallee());
    if (nullptr == llvmFunction) {
        std::cerr << "Error: Function is null" << std::endl;
        return -1;
    }

    NParameter* paramIt = params;
    for (auto it = llvmFunction->arg_begin(); it != llvmFunction->arg_end() && paramIt != nullptr; ++it) {
        it->setName(paramIt->getName());
        paramIt = paramIt->next;
    }

    context.functionDefinitions[name] = this;

    if (nullptr == body) {
        return 0;
    }

    context.currentFunction = this;
    context.isInitializingFunction = true;
    body->setName("entry");
    if (nullptr == body->codegen(context)) {
        return -1;
    }
    context.currentFunction = nullptr;
    context.isInitializingFunction = false;

    if (false != llvm::verifyFunction(*llvmFunction, &llvm::errs())) {
        return -1;
    }

    return 0;
}