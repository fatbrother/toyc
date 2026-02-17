#pragma once

#include "ast/node.hpp"
#include "ast/expression.hpp"

#include <map>

namespace toyc::ast {

class NStatement : public BasicNode {
public:
    virtual ~NStatement() override;
    virtual StmtCodegenResult codegen(ASTContext &context) = 0;
    virtual std::string getType() const override { return "Statement"; }
    void setParent(NStatement *parent) { this->parent = parent; }

public:
    NStatement *parent = nullptr;
    NStatement *next = nullptr;
};

class NDeclarationStatement : public NStatement, public NExternalDeclaration {
public:
    NDeclarationStatement(TypeIdx typeIdx, NDeclarator *declarator)
        : typeIdx(typeIdx), declarator(declarator) {}
    ~NDeclarationStatement() {
        SAFE_DELETE(declarator);
    }
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "DeclarationStatement"; }

private:
    StmtCodegenResult initializeArrayElements(
        llvm::AllocaInst* allocaInst,
        llvm::Type* arrayType,
        NInitializerList* initList,
        ASTContext& context);

    AllocCodegenResult createSingleAllocation(ASTContext &context, llvm::Type* type, NDeclarator* declarator);
    AllocCodegenResult createArrayAllocation(ASTContext &context, llvm::Type* baseType, NDeclarator* declarator);
    AllocCodegenResult createPointerAllocation(ASTContext &context, llvm::Type* baseType, NDeclarator* declarator);

    TypeIdx typeIdx;
    NDeclarator *declarator;
};

class NExpressionStatement : public NStatement {
public:
    NExpressionStatement(NExpression *expression) : expression(expression) {}
    ~NExpressionStatement() {
        SAFE_DELETE(expression);
    }
    virtual StmtCodegenResult codegen(ASTContext &context) override;
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
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "Block"; }
    void setName(const std::string &name) { this->name = name; }
    void setNextBlock(llvm::BasicBlock *nextBlock) { this->nextBlock = nextBlock; }
    NStatement* getStatements() const { return statements; }
    llvm::BasicBlock* getBlock() const { return block; }

private:
    std::string name;
    NStatement *statements;
    llvm::BasicBlock *nextBlock = nullptr;
    llvm::BasicBlock *block = nullptr;
};

class NReturnStatement : public NStatement {
public:
    NReturnStatement(NExpression *expression = nullptr) : expression(expression) {}
    ~NReturnStatement() {
        SAFE_DELETE(expression);
    }
    virtual StmtCodegenResult codegen(ASTContext &context) override;
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
    virtual StmtCodegenResult codegen(ASTContext &context) override;
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
    virtual StmtCodegenResult codegen(ASTContext &context) override;
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
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "WhileStatement"; }

private:
    NExpression *conditionNode;
    NBlock *bodyNode;
    bool isDoWhile; // true if this is a do-while loop
};

class NBreakStatement : public NStatement {
public:
    NBreakStatement() = default;
    ~NBreakStatement() = default;
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "BreakStatement"; }
};

class NContinueStatement : public NStatement {
public:
    NContinueStatement() = default;
    ~NContinueStatement() = default;
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "ContinueStatement"; }
};

class NLabelStatement : public NStatement {
public:
    NLabelStatement(const std::string& label, NStatement* statement)
        : label(label), statement(statement) {}
    ~NLabelStatement() {
        SAFE_DELETE(statement);
    }
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "LabelStatement"; }

private:
    std::string label;
    NStatement* statement;
};

class NGotoStatement : public NStatement {
public:
    NGotoStatement(const std::string& label) : label(label) {}
    ~NGotoStatement() = default;
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "GotoStatement"; }

private:
    std::string label;
};

class NSwitchStatement : public NStatement {
public:
    NSwitchStatement(NExpression *condition, NStatement *body)
        : condition(condition), body(body) {}
    ~NSwitchStatement() {
        SAFE_DELETE(condition);
        SAFE_DELETE(body);
    }
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "SwitchStatement"; }

private:
    NExpression *condition;
    NStatement *body;
};

class NCaseStatement : public NStatement {
public:
    NCaseStatement(NExpression *value, NStatement *statements = nullptr)
        : value(value), statements(statements), isDefault(false) {}

    NCaseStatement(bool isDefault, NStatement *statements = nullptr)
        : value(nullptr), statements(statements), isDefault(isDefault) {}

    ~NCaseStatement() {
        SAFE_DELETE(value);
        SAFE_DELETE(statements);
    }

    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "CaseStatement"; }

    NExpression* getValue() const { return value; }
    bool getIsDefault() const { return isDefault; }
    NStatement* getStatements() const { return statements; }

private:
    NExpression *value;
    NStatement *statements;
    bool isDefault;

    friend class NSwitchStatement;
};

} // namespace toyc::ast