#include "ast/external_definition.hpp"

#include <llvm/IR/Verifier.h>

#include <iostream>

#include "utility/raii_guard.hpp"

using namespace toyc::ast;

NFunctionDefinition::~NFunctionDefinition() {
    SAFE_DELETE(params);
    SAFE_DELETE(body);
}

StmtCodegenResult NFunctionDefinition::codegen(ASTContext &context) {
    returnType = context.typeManager->realize(returnTypeIdx);
    if (!returnType) {
        return StmtCodegenResult("Failed to realize return type");
    }

    std::vector<llvm::Type *> paramTypes;
    std::vector<std::string> paramNames;
    llvm::FunctionType *functionType = nullptr;
    bool isVariadic = false;

    for (NParameter *paramIt = params; paramIt != nullptr; paramIt = paramIt->next) {
        if (true == paramIt->isVariadic) {
            isVariadic = true;
            break;
        }

        llvm::Type *paramType = context.typeManager->realize(paramIt->getTypeIdx());
        if (nullptr == paramType) {
            return StmtCodegenResult("Failed to realize parameter type");
        }

        paramTypes.push_back(paramType);
        paramNames.push_back(paramIt->getName());
    }

    functionType = llvm::FunctionType::get(returnType, paramTypes, isVariadic);
    llvmFunction = static_cast<llvm::Function *>(context.module.getOrInsertFunction(name, functionType).getCallee());
    if (nullptr == llvmFunction) {
        return StmtCodegenResult("Function creation failed for " + name);
    }

    NParameter *paramIt = params;
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
    auto labelGuard = toyc::utility::makeScopeGuard([&context]() { context.clearLabels(); });
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
