#pragma once

#include "ast/node.hpp"
#include "ast/expression.hpp"
#include "ast/statement.hpp"

#include <iostream>

namespace toyc::ast {

class TypeDescriptor;

class NParameter : public BasicNode {
public:
    NParameter() : isVariadic(true), typeDesc(nullptr), name("") {}
    NParameter(TypeDescriptor *typeDesc, NDeclarator *declarator);

    ~NParameter();
    virtual std::string getType() const override { return "Parameter"; }

    TypeDescriptor* getTypeDescriptor() const { return typeDesc; }
    std::string getName() const { return name; }

public:
    NParameter *next = nullptr;
    bool isVariadic = false;

private:
    TypeDescriptor *typeDesc;
    std::string name;
};


class NFunctionDefinition : public NExternalDeclaration {
public:
    NFunctionDefinition(TypeDescriptor *returnTypeDesc, const std::string &name, NParameter *params, NBlock *body)
        : returnTypeDesc(returnTypeDesc), name(name), params(params), body(body) {}
    ~NFunctionDefinition();
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "FunctionDefinition"; }
    llvm::Function *getFunction() const { return llvmFunction; }
    llvm::Type* getReturnType() const { return returnType; }
    NParameter *getParams() const { return params; }
    NBlock *getBody() const { return body; }

private:
    llvm::Function *llvmFunction = nullptr;
    std::string name;
    TypeDescriptor *returnTypeDesc;
    llvm::Type* returnType;
    NParameter *params;
    NBlock *body;
};

} // namespace toyc::ast