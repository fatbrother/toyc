#pragma once

#include <ast/node.hpp>

#include <iostream>

namespace toyc::ast {

class NBinaryOperator : public NExpression {
public:
    NBinaryOperator(NExpression *lhs, BineryOperator op, NExpression *rhs)
        : lhs(lhs), rhs(rhs), op(op) {}
    ~NBinaryOperator() {
        SAFE_DELETE(lhs);
        SAFE_DELETE(rhs);
    }
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) override;
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
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) override;
    virtual std::string getType() const override { return "UnaryOperator"; }
    virtual llvm::AllocaInst *allocgen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) override {
        return expr->allocgen(context, module, builder, parent);
    };

private:
    UnaryOperator op;
    NExpression *expr;
};

class NIdentifier : public NExpression {
public:
    NIdentifier(const std::string &name) : name(name) {}
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) override;
    virtual llvm::AllocaInst *allocgen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) override;
    virtual std::string getType() const override { return "Identifier"; }

private:
    std::string name;
};

class NInteger : public NExpression {
public:
    NInteger(int value) : value(value) {}
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) override;
    virtual std::string getType() const override { return "Integer"; }

private:
    int value;
};

class NFloat : public NExpression {
public:
    NFloat(double value) : value(value) {}
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) override;
    virtual std::string getType() const override { return "Float"; }

private:
    double value;
};

class NString : public NExpression {
public:
    NString(const std::string &value) : value(value) {}
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) override;
    virtual std::string getType() const override { return "String"; }

private:
    std::string value;
};

class NDeclarator : public NExpression {
public:
    NDeclarator(const std::string &name, NExpression *expr)
        : name(name), expr(expr) {}
    ~NDeclarator() {
        SAFE_DELETE(expr);
        SAFE_DELETE(next);
    }
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) override {
        if (nullptr == expr) {
            return nullptr;
        }
        return expr->codegen(context, module, builder, parent);
    }
    virtual std::string getType() const override { return "Declaration"; }
    std::string getName() const { return name; }

public:
    NDeclarator *next = nullptr;

private:
    std::string name;
    NExpression *expr;
};

class NAssignment : public NExpression {
public:
    NAssignment(NExpression *lhs, NExpression *rhs)
        : lhs(lhs), rhs(rhs) {}
    ~NAssignment() {
        SAFE_DELETE(lhs);
        SAFE_DELETE(rhs);
    }
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) override;
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
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) override;
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
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) override;
    virtual std::string getType() const override { return "FunctionCall"; }

private:
    std::string name;
    NArguments *argNodes;
};

} // namespace toyc::ast