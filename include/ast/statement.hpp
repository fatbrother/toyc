#pragma once

#include "ast/node.hpp"
#include "ast/expression.hpp"

#include <map>

namespace toyc::ast {

class NDeclarationStatement : public NStatement
{
public:
    NDeclarationStatement(NType *type, NDeclarator *declarator)
        : type(type), declarator(declarator) {}
    ~NDeclarationStatement()
    {
        delete type;
        delete declarator;
    }
    virtual void codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) override;
    virtual std::string getType() const override { return "DeclarationStatement"; }

private:
    NType *type;
    NDeclarator *declarator;
};

class NExpressionStatement : public NStatement
{
public:
    NExpressionStatement(NExpression *expression) : expression(expression) {}
    ~NExpressionStatement() { delete expression; }
    virtual void codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) override
    {
        if (nullptr != expression)
        {
            expression->codegen(context, module, builder, parent);
        }
    }
    virtual std::string getType() const override { return "ExpressionStatement"; }

private:
    NExpression *expression;
};

class NBlock : public NStatement
{
public:
    NBlock(NStatement *statements = nullptr) : statements(statements) {}
    ~NBlock()
    {
        if (statements)
        {
            delete statements;
        }
        if (next)
        {
            delete next;
        }
    }
    virtual void codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) override;
    virtual std::string getType() const override { return "Block"; }
    void setParentFunction(llvm::Function *parent) { this->parentFunction = parent; }
    void setName(const std::string &name) { this->name = name; }
    llvm::AllocaInst *findVariable(const std::string &name);

public:
    std::map<std::string, llvm::AllocaInst *> variables;

private:
    std::string name;
    NStatement *statements;
    llvm::Function *parentFunction = nullptr;
};

class NReturnStatement : public NStatement {
public:
    NReturnStatement(NExpression *expression = nullptr) : expression(expression) {}
    ~NReturnStatement() {
        if (expression) {
            delete expression;
        }
    }
    virtual void codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) override;
    virtual std::string getType() const override { return "ReturnStatement"; }

private:
    NExpression *expression;
};


} // namespace toyc::ast