#include "ast/external_definition.hpp"
#include "utility/raii_guard.hpp"

#include <iostream>
#include <llvm/IR/Verifier.h>

using namespace toyc::ast;

NParameter::NParameter(NType *type, NDeclarator *declarator)
    : isVariadic(false), type(nullptr), name("") {
    if (declarator) {
        name = declarator->getName();

        if (declarator->isArray()) {
            this->type = std::make_shared<NPointerType>(std::shared_ptr<NType>(type), 1);
        } else if (declarator->pointerLevel > 0) {
            this->type = std::make_shared<NPointerType>(std::shared_ptr<NType>(type), declarator->pointerLevel);
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

StmtCodegenResult NFunctionDefinition::codegen(ASTContext &context) {
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

        TypeCodegenResult paramTypeResult = paramIt->getVarType()->getLLVMType(context);
        if (false == paramTypeResult.isSuccess()) {
            return StmtCodegenResult("Failed to get LLVM type for parameter") << paramTypeResult;
        }
        llvm::Type *paramType = paramTypeResult.getLLVMType();
        if (nullptr == paramType) {
            return StmtCodegenResult("Parameter type is null");
        }

        paramTypes.push_back(paramType);
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