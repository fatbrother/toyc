#pragma once

#include "ast/node.hpp"
#include "ast/expression.hpp"
#include "ast/statement.hpp"

#include <iostream>

namespace toyc::ast {

class NParameter : public BasicNode {
public:
    NParameter() : isVariadic(true), type(nullptr), name("") {}
    NParameter(NType *type, NDeclarator *declarator);

    ~NParameter() {
        SAFE_DELETE(next);
    }
    virtual std::string getType() const override { return "Parameter"; }
    llvm::Type *getLLVMType(llvm::LLVMContext &context) const {
        return type->getLLVMType(context);
    }
    NTypePtr getVarType() const {
        return type;
    }
    std::string getName() const { return name; }

public:
    NParameter *next = nullptr;
    bool isVariadic = false;

private:
    NTypePtr type;
    std::string name;
};


class NFunctionDefinition : public NExternalDeclaration {
public:
    NFunctionDefinition(NType *returnType, const std::string &name, NParameter *params, NBlock *body)
        : returnType(returnType), name(name), params(params), body(body) {}
    ~NFunctionDefinition();
    virtual llvm::Value *codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "FunctionDefinition"; }
    llvm::Function *getFunction() const { return llvmFunction; }
    NTypePtr getReturnType() const { return returnType; }
    NParameter *getParams() const { return params; }

private:
    llvm::Function *llvmFunction = nullptr;
    std::string name;
    NTypePtr returnType;
    NParameter *params;
    NBlock *body;
};

} // namespace toyc::ast