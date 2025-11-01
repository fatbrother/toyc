#pragma once

#include "ast/node.hpp"
#include "ast/expression.hpp"

#include <map>

namespace toyc::ast {

class NStatement : public BasicNode {
public:
    virtual ~NStatement() override;
    virtual CodegenResult codegen(ASTContext &context) = 0;
    virtual std::string getType() const override { return "Statement"; }
    void setParent(NStatement *parent) { this->parent = parent; }

public:
    NStatement *parent = nullptr;
    NStatement *next = nullptr;
};

class NDeclarationStatement : public NStatement, public NExternalDeclaration {
public:
    NDeclarationStatement(NType *type, NDeclarator *declarator)
        : type(type), declarator(declarator) {}
    ~NDeclarationStatement() {
        SAFE_DELETE(declarator);
    }
    virtual CodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "DeclarationStatement"; }

private:
    NTypePtr type;
    NDeclarator *declarator;
};

class NExpressionStatement : public NStatement {
public:
    NExpressionStatement(NExpression *expression) : expression(expression) {}
    ~NExpressionStatement() {
        SAFE_DELETE(expression);
    }
    virtual CodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "ExpressionStatement"; }

private:
    NExpression *expression;
};

class NBlock : public NStatement
{
public:
    NBlock(NStatement *statements = nullptr) : statements(statements) {}
    ~NBlock() {
        SAFE_DELETE(statements);
    }
    virtual CodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "Block"; }
    void setName(const std::string &name) { this->name = name; }
    void setNextBlock(llvm::BasicBlock *nextBlock) { this->nextBlock = nextBlock; }

private:
    std::string name;
    NStatement *statements;
    llvm::BasicBlock *nextBlock = nullptr;
};

class NReturnStatement : public NStatement {
public:
    NReturnStatement(NExpression *expression = nullptr) : expression(expression) {}
    ~NReturnStatement() {
        SAFE_DELETE(expression);
    }
    virtual CodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "ReturnStatement"; }

private:
    NExpression *expression;
};

class NIfStatement : public NStatement {
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
    virtual CodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "IfStatement"; }

private:
    NExpression *conditionNode;
    NBlock *thenBlockNode;
    NBlock *elseBlockNode;
};

class NForStatement : public NStatement {
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
    virtual CodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "ForStatement"; }

private:
    NStatement *initializationNode;
    NExpression *conditionNode;
    NExpression *incrementNode;
    NBlock *bodyNode;
};

class NWhileStatement : public NStatement {
public:
    NWhileStatement(NExpression *conditionNode, NStatement *bodyNode, bool isDoWhile = false)
        : conditionNode(conditionNode), isDoWhile(isDoWhile) {
            if ("Block" == bodyNode->getType()) {
                this->bodyNode = static_cast<NBlock *>(bodyNode);
            } else {
                this->bodyNode = new NBlock(bodyNode);
            }
        }
    ~NWhileStatement() {
        SAFE_DELETE(conditionNode);
        SAFE_DELETE(bodyNode);
    }
    virtual CodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "WhileStatement"; }

private:
    NExpression *conditionNode;
    NBlock *bodyNode;
    bool isDoWhile; // true if this is a do-while loop
};

} // namespace toyc::ast