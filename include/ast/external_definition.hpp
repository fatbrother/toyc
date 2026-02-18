#pragma once

#include <iostream>
#include <memory>

#include "ast/expression.hpp"
#include "ast/node.hpp"
#include "ast/statement.hpp"

namespace toyc::ast {

class NParameter : public BasicNode {
public:
    NParameter() : isVariadic(true), typeIdx(InvalidTypeIdx), name("") {}
    NParameter(TypeIdx typeIdx, std::string name, NDeclarator *declarator)
        : isVariadic(false), typeIdx(typeIdx), name(std::move(name)) {
        // declarator is consumed and no longer needed
        delete declarator;
    }

    virtual std::string getType() const override { return "Parameter"; }

    TypeIdx getTypeIdx() const { return typeIdx; }
    std::string getName() const { return name; }

public:
    std::unique_ptr<NParameter> next;
    bool isVariadic = false;

private:
    TypeIdx typeIdx;
    std::string name;
};

class NFunctionDefinition : public NExternalDeclaration {
public:
    NFunctionDefinition(TypeIdx returnTypeIdx, const std::string &name, NParameter *params, NBlock *body)
        : name(name), returnTypeIdx(returnTypeIdx), params(params), body(body) {}
    ~NFunctionDefinition();
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "FunctionDefinition"; }
    llvm::Function *getFunction() const { return llvmFunction; }
    llvm::Type *getReturnType() const { return returnType; }
    TypeIdx getReturnTypeIdx() const { return returnTypeIdx; }
    NParameter *getParams() const { return params.get(); }
    NBlock *getBody() const { return body.get(); }

private:
    llvm::Function *llvmFunction = nullptr;
    std::string name;
    TypeIdx returnTypeIdx;
    llvm::Type *returnType = nullptr;
    std::unique_ptr<NParameter> params;
    std::unique_ptr<NBlock> body;
};

}  // namespace toyc::ast
