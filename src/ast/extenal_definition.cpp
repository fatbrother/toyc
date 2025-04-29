#include "ast/node.hpp"
#include "ast/statement.hpp"
#include "ast/external_definition.hpp"

#include "parser/y.tab.hpp"

#include <iostream>
#include <llvm/IR/Verifier.h>

using namespace toyc::ast;

void NFunctionDefinition::codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) {
    unsigned int index = 0;
    llvm::Type *llvmReturnType = returnType->getLLVMType(context);
    std::vector<llvm::Type *> paramTypes;
    std::vector<std::string> paramNames;
    NParameter *param = nullptr;
    llvm::Function *function = module.getFunction(name);

    if (nullptr == function) {
        for (param = params; param != nullptr; param = param->next) {
            llvm::Type *paramType = param->getLLVMType(context);
            if (nullptr == paramType) {
                std::cerr << "Error: Parameter type is null" << std::endl;
                return;
            }

            paramTypes.push_back(paramType);
            paramNames.push_back(param->getName());
        }

        llvm::FunctionType *functionType = llvm::FunctionType::get(llvmReturnType, paramTypes, false);
        function = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, name, module);

        param = params;
        for (auto it = function->arg_begin(), end = function->arg_end(); it != end; ++it) {
            it->setName(param->getName());
            param = param->next;
        }
    }

    if (nullptr == body) {
        std::cerr << "Error: Function body is null" << std::endl;
        return;
    }

    body->setName("body");
    body->setParentFunction(function);
    body->codegen(context, module, builder);

    if (llvmReturnType->isVoidTy()) {
        builder.CreateRetVoid();
    }

    if (false == llvm::verifyFunction(*function)) {
        std::cerr << "Error: Function verification failed" << std::endl;
        return;
    }
}