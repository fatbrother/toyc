#pragma once

#include "ast/node.hpp"
#include "ast/statement.hpp"

namespace toyc::ast {

class NParameter : public BasicNode {
public:
    NParameter(NType *type, const std::string &name)
        : type(type), name(name) {}
    ~NParameter() {
        delete type;
    }
    virtual std::string getType() const override { return "Parameter"; }
    llvm::Type *getLLVMType(llvm::LLVMContext &context) const {
        return type->getLLVMType(context);
    }
    std::string getName() const { return name; }

public:
    NParameter *next = nullptr;

private:
    NType *type;
    std::string name;
};


class NFunctionDefinition : public NExternalDeclaration {
public:
    NFunctionDefinition(NType *returnType, const std::string &name, NParameter *params, NBlock *body)
        : returnType(returnType), name(name), params(params), body(body) {}
    ~NFunctionDefinition() {
        delete returnType;
        delete params;
        delete body;
    }
    virtual void codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) override;
    virtual std::string getType() const override { return "FunctionDefinition"; }

private:
    NType *returnType;
    std::string name;
    NParameter *params;
    NBlock *body;
};

} // namespace toyc::ast