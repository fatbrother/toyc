#pragma once

#include <map>
#include <memory>

#include "ast/expression.hpp"
#include "ast/node.hpp"

namespace toyc::ast {

class NStatement : public BasicNode {
public:
    virtual ~NStatement() override = default;
    virtual StmtCodegenResult codegen(ASTContext &context) = 0;
    virtual std::string getType() const override { return "Statement"; }
    void setParent(NStatement *parent) { this->parent = parent; }

public:
    NStatement *parent = nullptr;
    std::unique_ptr<NStatement> next;
};

class NDeclarationStatement : public NStatement, public NExternalDeclaration {
public:
    NDeclarationStatement(TypeIdx typeIdx, NDeclarator *declarator) : typeIdx(typeIdx), declarator(declarator) {}
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "DeclarationStatement"; }

private:
    StmtCodegenResult initializeArrayElements(llvm::AllocaInst *allocaInst, TypeIdx arrayTypeIdx,
                                              NInitializerList *initList, ASTContext &context);

    AllocCodegenResult createSingleAllocation(ASTContext &context, llvm::Type *type, TypeIdx typeIdx,
                                              NDeclarator *declarator);
    AllocCodegenResult createArrayAllocation(ASTContext &context, llvm::Type *baseType, TypeIdx baseTypeIdx,
                                             NDeclarator *declarator);
    AllocCodegenResult createPointerAllocation(ASTContext &context, llvm::Type *baseType, TypeIdx baseTypeIdx,
                                               NDeclarator *declarator);

    TypeIdx typeIdx;
    std::unique_ptr<NDeclarator> declarator;
};

class NExpressionStatement : public NStatement {
public:
    explicit NExpressionStatement(NExpression *expression) : expression(expression) {}
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "ExpressionStatement"; }

private:
    std::unique_ptr<NExpression> expression;
};

class NBlock : public NStatement {
public:
    explicit NBlock(NStatement *statements = nullptr) : statements(statements) {}
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "Block"; }
    void setName(const std::string &name) { this->name = name; }
    void setNextBlock(llvm::BasicBlock *nextBlock) { this->nextBlock = nextBlock; }
    NStatement *getStatements() const { return statements.get(); }
    llvm::BasicBlock *getBlock() const { return block; }

private:
    std::string name;
    std::unique_ptr<NStatement> statements;
    llvm::BasicBlock *nextBlock = nullptr;
    llvm::BasicBlock *block = nullptr;
};

class NReturnStatement : public NStatement {
public:
    explicit NReturnStatement(NExpression *expression = nullptr) : expression(expression) {}
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "ReturnStatement"; }

private:
    std::unique_ptr<NExpression> expression;
};

class NIfStatement : public NStatement {
public:
    NIfStatement(NExpression *conditionNode, NStatement *thenBlockNode, NStatement *elseBlockNode = nullptr)
        : conditionNode(conditionNode) {
        if ("Block" == thenBlockNode->getType()) {
            this->thenBlockNode.reset(static_cast<NBlock *>(thenBlockNode));
        } else {
            this->thenBlockNode = std::make_unique<NBlock>(thenBlockNode);
        }
        if (nullptr != elseBlockNode) {
            if ("Block" == elseBlockNode->getType()) {
                this->elseBlockNode.reset(static_cast<NBlock *>(elseBlockNode));
            } else {
                this->elseBlockNode = std::make_unique<NBlock>(elseBlockNode);
            }
        }
    }
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "IfStatement"; }

private:
    std::unique_ptr<NExpression> conditionNode;
    std::unique_ptr<NBlock> thenBlockNode;
    std::unique_ptr<NBlock> elseBlockNode;
};

class NForStatement : public NStatement {
public:
    NForStatement(NStatement *initializationNode, NExpression *conditionNode, NExpression *incrementNode,
                  NStatement *body)
        : initializationNode(initializationNode), conditionNode(conditionNode), incrementNode(incrementNode) {
        if ("Block" == body->getType()) {
            bodyNode.reset(static_cast<NBlock *>(body));
        } else {
            bodyNode = std::make_unique<NBlock>(body);
        }
    }
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "ForStatement"; }

private:
    std::unique_ptr<NStatement> initializationNode;
    std::unique_ptr<NExpression> conditionNode;
    std::unique_ptr<NExpression> incrementNode;
    std::unique_ptr<NBlock> bodyNode;
};

class NWhileStatement : public NStatement {
public:
    NWhileStatement(NExpression *conditionNode, NStatement *bodyNode, bool isDoWhile = false)
        : conditionNode(conditionNode), isDoWhile(isDoWhile) {
        if ("Block" == bodyNode->getType()) {
            this->bodyNode.reset(static_cast<NBlock *>(bodyNode));
        } else {
            this->bodyNode = std::make_unique<NBlock>(bodyNode);
        }
    }
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "WhileStatement"; }

private:
    std::unique_ptr<NExpression> conditionNode;
    std::unique_ptr<NBlock> bodyNode;
    bool isDoWhile;  // true if this is a do-while loop
};

class NBreakStatement : public NStatement {
public:
    NBreakStatement() = default;
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "BreakStatement"; }
};

class NContinueStatement : public NStatement {
public:
    NContinueStatement() = default;
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "ContinueStatement"; }
};

class NLabelStatement : public NStatement {
public:
    NLabelStatement(const std::string &label, NStatement *statement) : label(label), statement(statement) {}
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "LabelStatement"; }

private:
    std::string label;
    std::unique_ptr<NStatement> statement;
};

class NGotoStatement : public NStatement {
public:
    explicit NGotoStatement(const std::string &label) : label(label) {}
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "GotoStatement"; }

private:
    std::string label;
};

class NSwitchStatement : public NStatement {
public:
    NSwitchStatement(NExpression *condition, NStatement *body) : condition(condition), body(body) {}
    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "SwitchStatement"; }

private:
    std::unique_ptr<NExpression> condition;
    std::unique_ptr<NStatement> body;
};

class NCaseStatement : public NStatement {
public:
    explicit NCaseStatement(NExpression *value, NStatement *statements = nullptr)
        : value(value), statements(statements), isDefault(false) {}

    explicit NCaseStatement(bool isDefault, NStatement *statements = nullptr)
        : value(nullptr), statements(statements), isDefault(isDefault) {}

    virtual StmtCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "CaseStatement"; }

    NExpression *getValue() const { return value.get(); }
    bool getIsDefault() const { return isDefault; }
    NStatement *getStatements() const { return statements.get(); }

private:
    std::unique_ptr<NExpression> value;
    std::unique_ptr<NStatement> statements;
    bool isDefault;

    friend class NSwitchStatement;
};

}  // namespace toyc::ast
