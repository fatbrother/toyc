#pragma once

#include <ast/node.hpp>

#include <iostream>

namespace toyc::ast {

class NExpression : public BasicNode {
public:
    virtual std::pair<llvm::Value *, NTypePtr> codegen(ASTContext &context) {
        throw std::runtime_error("Code generation not implemented for this expression type");
    }
    virtual std::pair<llvm::AllocaInst *, NTypePtr> allocgen(ASTContext &context) {
        throw std::runtime_error("Expression is not a valid left value");
    };
    virtual std::string getType() const override { return "Expression"; }
};

class NBinaryOperator : public NExpression {
public:
    NBinaryOperator(NExpression *lhs, BineryOperator op, NExpression *rhs)
        : lhs(lhs), rhs(rhs), op(op) {}
    ~NBinaryOperator() {
        SAFE_DELETE(lhs);
        SAFE_DELETE(rhs);
    }
    virtual std::pair<llvm::Value *, NTypePtr> codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "BinaryOperator"; }

private:
    NExpression *lhs;
    NExpression *rhs;
    BineryOperator op;
};

class NUnaryExpression : public NExpression {
public:
    NUnaryExpression(UnaryOperator op, NExpression *expr)
        : op(op), expr(expr) {}
    ~NUnaryExpression() {
        SAFE_DELETE(expr);
    }
    virtual std::pair<llvm::Value *, NTypePtr> codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "UnaryOperator"; }
    virtual std::pair<llvm::AllocaInst *, NTypePtr> allocgen(ASTContext &context) override {
        return expr->allocgen(context);
    };

private:
    UnaryOperator op;
    NExpression *expr;
};

class NConditionalExpression : public NExpression {
public:
    NConditionalExpression(NExpression *condition, NExpression *trueExpr, NExpression *falseExpr)
        : condition(condition), trueExpr(trueExpr), falseExpr(falseExpr) {}
    ~NConditionalExpression() {
        SAFE_DELETE(condition);
        SAFE_DELETE(trueExpr);
        SAFE_DELETE(falseExpr);
    }
    virtual std::pair<llvm::Value *, NTypePtr> codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "ConditionalExpression"; }

private:
    NExpression *condition;
    NExpression *trueExpr;
    NExpression *falseExpr;
};

class NIdentifier : public NExpression {
public:
    NIdentifier(const std::string &name) : name(name) {}
    virtual std::pair<llvm::Value *, NTypePtr> codegen(ASTContext &context) override;
    virtual std::pair<llvm::AllocaInst *, NTypePtr> allocgen(ASTContext &context) override;
    virtual std::string getType() const override { return "Identifier"; }

private:
    std::string name;
};

class NInteger : public NExpression {
public:
    NInteger(int value) : value(value) {}
    virtual std::pair<llvm::Value *, NTypePtr> codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "Integer"; }

private:
    int value;
};

class NFloat : public NExpression {
public:
    NFloat(double value) : value(value) {}
    virtual std::pair<llvm::Value *, NTypePtr> codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "Float"; }

private:
    double value;
};

class NString : public NExpression {
public:
    NString(const std::string &value) : value(value) {}
    virtual std::pair<llvm::Value *, NTypePtr> codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "String"; }

private:
    std::string value;
};

class NDeclarator : public NExpression {
public:
    NDeclarator(const std::string &name, int pointerLevel = 0)
        : name(name), pointerLevel(pointerLevel) {}
    ~NDeclarator() {
        SAFE_DELETE(expr);
        SAFE_DELETE(next);
    }
    virtual std::pair<llvm::Value *, NTypePtr> codegen(ASTContext &context) override {
        if (nullptr == expr) {
            return std::make_pair(nullptr, nullptr);
        }
        return expr->codegen(context);
    }
    virtual std::string getType() const override { return "Declaration"; }
    std::string getName() const { return name; }
    bool isNonInitialized() const { return expr == nullptr; }

public:
    int pointerLevel = 0;
    NDeclarator *next = nullptr;
    NExpression *expr = nullptr;

private:
    std::string name;
};

class NAssignment : public NExpression {
public:
    NAssignment(NExpression *lhs, NExpression *rhs)
        : lhs(lhs), rhs(rhs) {}
    ~NAssignment() {
        SAFE_DELETE(lhs);
        SAFE_DELETE(rhs);
    }
    virtual std::pair<llvm::Value *, NTypePtr> codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "Assignment"; }

private:
    NExpression *lhs;
    NExpression *rhs;
};

class NArguments : public NExpression {
public:
    NArguments(NExpression *expr) : expr(expr) {}
    ~NArguments() {
        SAFE_DELETE(expr);
        SAFE_DELETE(next);
    }
    virtual std::pair<llvm::Value *, NTypePtr> codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "Arguments"; }

public:
    NArguments *next = nullptr;
    NExpression *expr = nullptr;
};

class NFunctionCall : public NExpression {
public:
    NFunctionCall(const std::string &name, NArguments *argNodes)
        : name(name), argNodes(argNodes) {}
    ~NFunctionCall() {
        SAFE_DELETE(argNodes);
    }
    virtual std::pair<llvm::Value *, NTypePtr> codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "FunctionCall"; }

private:
    std::string name;
    NArguments *argNodes;
};

} // namespace toyc::ast