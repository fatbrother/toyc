#pragma once

#include "ast/node.hpp"
#include "ast/statement.hpp"

namespace toyc::ast {

class NParameter : public BasicNode {
public:
    NParameter(NType *type, const std::string &name, bool isVarArg = false)
        : type(type), name(name), isVarArg(isVarArg) {}
    ~NParameter() {
        SAFE_DELETE(type);
        SAFE_DELETE(next);
    }
    virtual std::string getType() const override { return "Parameter"; }
    llvm::Type *getLLVMType(llvm::LLVMContext &context) const {
        return type->getLLVMType(context);
    }
    std::string getName() const { return name; }

public:
    NParameter *next = nullptr;
    bool isVarArg = false;

private:
    NType *type;
    std::string name;
};


class NFunctionDefinition : public NExternalDeclaration {
public:
    NFunctionDefinition(NType *returnType, const std::string &name, NParameter *params, NBlock *body)
        : returnType(returnType), name(name), params(params), body(body) {}
    ~NFunctionDefinition() {
        SAFE_DELETE(returnType);
        SAFE_DELETE(params);
        SAFE_DELETE(body);
    }
    virtual void codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) override;
    virtual std::string getType() const override { return "FunctionDefinition"; }

private:
    std::string name;
    NType *returnType;
    NParameter *params;
    NBlock *body;
};

} // namespace toyc::ast