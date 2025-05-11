#pragma once

#include "ast/node.hpp"
#include "ast/expression.hpp"

#include <map>

namespace toyc::ast {

class NStatement : public BasicNode {
public:
    virtual ~NStatement() override;
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) = 0;
    virtual std::string getType() const override { return "Statement"; }
    void setParent(NParentStatement *parent) { this->parent = parent; }

public:
    NParentStatement *parent = nullptr;
    NStatement *next = nullptr;
};

class NParentStatement : public NStatement {
public:
    llvm::AllocaInst *lookupVariable(const std::string &name);
    void insertVariable(const std::string &name, llvm::AllocaInst *allocaInst);

protected:
    std::map<std::string, llvm::AllocaInst *> variables;
};

class NDeclarationStatement : public NStatement {
public:
    NDeclarationStatement(NType *type, NDeclarator *declarator)
        : type(type), declarator(declarator) {}
    ~NDeclarationStatement()
    {
        SAFE_DELETE(type);
        SAFE_DELETE(declarator);
    }
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) override;
    virtual std::string getType() const override { return "DeclarationStatement"; }

private:
    NType *type;
    NDeclarator *declarator;
};

class NExpressionStatement : public NStatement {
public:
    NExpressionStatement(NExpression *expression) : expression(expression) {}
    ~NExpressionStatement() { delete expression; }
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) override {
        if (nullptr != expression) {
            return expression->codegen(context, module, builder, parent);
        }
        return nullptr;
    }
    virtual std::string getType() const override { return "ExpressionStatement"; }

private:
    NExpression *expression;
};

class NBlock : public NParentStatement
{
public:
    NBlock(NStatement *statements = nullptr) : statements(statements) {}
    ~NBlock() {
        SAFE_DELETE(statements);
    }
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) override;
    virtual std::string getType() const override { return "Block"; }
    void setParentFunction(llvm::Function *parent) { this->parentFunction = parent; }
    void setName(const std::string &name) { this->name = name; }
    void setNextBlock(llvm::BasicBlock *nextBlock) { this->nextBlock = nextBlock; }
    void setIsFunctionBlock(bool isFunctionBlock) { this->isFunctionBlock = isFunctionBlock; }

private:
    bool isFunctionBlock = false;
    std::string name;
    NStatement *statements;
    llvm::BasicBlock *nextBlock = nullptr;
    llvm::Function *parentFunction = nullptr;
};

class NReturnStatement : public NStatement {
public:
    NReturnStatement(NExpression *expression = nullptr) : expression(expression) {}
    ~NReturnStatement() {
        SAFE_DELETE(expression);
    }
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) override;
    virtual std::string getType() const override { return "ReturnStatement"; }

private:
    NExpression *expression;
};

class NIfStatement : public NParentStatement {
public:
    NIfStatement(NExpression *conditionNode, NStatement *thenBlockNode, NStatement *elseBlockNode = nullptr)
        : conditionNode(conditionNode) {
            if ("Block" == thenBlockNode->getType()) {
                this->thenBlockNode = static_cast<NBlock *>(thenBlockNode);
            } else {
                this->thenBlockNode = new NBlock(thenBlockNode);
            }
            if (nullptr != elseBlockNode) {
                if ("Block" == elseBlockNode->getType()) {
                    this->elseBlockNode = static_cast<NBlock *>(elseBlockNode);
                } else {
                    this->elseBlockNode = new NBlock(elseBlockNode);
                }
            } else {
                this->elseBlockNode = nullptr;
            }
        }
    ~NIfStatement() {
        SAFE_DELETE(conditionNode);
        SAFE_DELETE(thenBlockNode);
        SAFE_DELETE(elseBlockNode);
    }
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) override;
    virtual std::string getType() const override { return "IfStatement"; }

private:
    NExpression *conditionNode;
    NBlock *thenBlockNode;
    NBlock *elseBlockNode;
};

class NForStatement : public NParentStatement {
public:
    NForStatement(NStatement *initializationNode, NExpression *conditionNode, NExpression *incrementNode, NStatement *body)
        : initializationNode(initializationNode), conditionNode(conditionNode), incrementNode(incrementNode) {
            if ("Block" == body->getType()) {
                bodyNode = static_cast<NBlock *>(body);
            } else {
                bodyNode = new NBlock(body);
            }
        }
    ~NForStatement() {
        SAFE_DELETE(initializationNode);
        SAFE_DELETE(conditionNode);
        SAFE_DELETE(incrementNode);
        SAFE_DELETE(bodyNode);
    }
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) override;
    virtual std::string getType() const override { return "ForStatement"; }

private:
    NStatement *initializationNode;
    NExpression *conditionNode;
    NExpression *incrementNode;
    NBlock *bodyNode;
};

} // namespace toyc::ast