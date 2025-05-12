#include "ast/external_definition.hpp"

#include <iostream>
#include <llvm/IR/Verifier.h>

using namespace toyc::ast;

void NFunctionDefinition::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) {
    unsigned int index = 0;
    llvm::Type *llvmReturnType = returnType->getLLVMType(context);
    std::vector<llvm::Type *> paramTypes;
    std::vector<std::string> paramNames;
    NParameter *paramIt = nullptr;
    llvm::Function *function = nullptr;
    llvm::FunctionType *functionType = nullptr;

    for (NParameter *paramIt = params; paramIt != nullptr; paramIt = paramIt->next) {
        llvm::Type *paramType = paramIt->getLLVMType(context);
        if (nullptr == paramType) {
            std::cerr << "Error: Parameter type is null" << std::endl;
            return;
        }

        paramTypes.push_back(paramType);
        paramNames.push_back(paramIt->getName());
    }

    functionType = llvm::FunctionType::get(llvmReturnType, paramTypes, false);
    function = static_cast<llvm::Function *>(module.getOrInsertFunction(name, functionType).getCallee());
    if (nullptr == function) {
        std::cerr << "Error: Function is null" << std::endl;
        return;
    }

    paramIt = params;
    for (auto it = function->arg_begin(); it != function->arg_end(); ++it) {
        it->setName(paramIt->getName());
        paramIt = paramIt->next;
    }

    if (nullptr == body) {
        return;
    }

    body->setName("entry");
    body->setParentFunction(function);
    body->codegen(context, module, builder);

    if (false != llvm::verifyFunction(*function, &llvm::errs())) {
        return;
    }
}