#pragma once

#include "ast/node.hpp"
#include "ast/expression.hpp"
#include "ast/statement.hpp"

#include <iostream>

namespace toyc::ast {

class NParameter : public BasicNode {
public:
    NParameter() : isVariadic(true), typeIdx(InvalidTypeIdx), name("") {}
    NParameter(TypeIdx typeIdx, std::string name, NDeclarator *declarator)
        : isVariadic(false), typeIdx(typeIdx), name(std::move(name)) {
        SAFE_DELETE(declarator);
    }

    ~NParameter() { SAFE_DELETE(next); }
    virtual std::string getType() const override { return "Parameter"; }

    TypeIdx getTypeIdx() const { return typeIdx; }
    std::string getName() const { return name; }

public:
    NParameter *next = nullptr;
    bool isVariadic = false;

private:
    TypeIdx typeIdx;
    std::string name;
};


class NFunctionDefinition : public NExternalDeclaration {
public:
    NFunctionDefinition(TypeIdx returnTypeIdx, const std::string &name, NParameter *params, NBlock *body)
        : returnTypeIdx(returnTypeIdx), name(name), params(params), body(body) {}
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
    TypeIdx returnTypeIdx;
    llvm::Type* returnType = nullptr;
    NParameter *params;
    NBlock *body;
};

} // namespace toyc::ast