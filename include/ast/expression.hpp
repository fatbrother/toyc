#pragma once

#include <ast/node.hpp>

namespace toyc::ast {

class NExpression : public BasicNode {
public:
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NBlock *parent) = 0;
    virtual std::string getType() const override { return "Expression"; }
};

class NBinaryOperator : public NExpression {
public:
    NBinaryOperator(NExpression *lhs, int &op, NExpression *rhs)
        : lhs(lhs), rhs(rhs), op(op) {}
    ~NBinaryOperator() {
        delete lhs;
        delete rhs;
    }
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NBlock *parent) override;
    virtual std::string getType() const override { return "BinaryOperator"; }

private:
    NExpression *lhs;
    NExpression *rhs;
    int op;
};

class NIdentifier : public NExpression {
public:
    NIdentifier(const std::string &name) : name(name) {}
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NBlock *parent) override;
    virtual std::string getType() const override { return "Identifier"; }

private:
    std::string name;
};

class NInteger : public NExpression {
public:
    NInteger(int value) : value(value) {}
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NBlock *parent) override;
    virtual std::string getType() const override { return "Integer"; }

private:
    int value;
};

class NFloat : public NExpression {
public:
    NFloat(double value) : value(value) {}
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NBlock *parent) override;
    virtual std::string getType() const override { return "Float"; }

private:
    double value;
};

class NString : public NExpression {
public:
    NString(const std::string &value) : value(value) {}
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NBlock *parent) override;
    virtual std::string getType() const override { return "String"; }

private:
    std::string value;
};

class NDeclarator : public NExpression {
public:
    NDeclarator(const std::string &name, NExpression *expr)
        : name(name), expr(expr) {}
    ~NDeclarator() {
        delete expr;
        if (next) {
            delete next;
        }
    }
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NBlock *parent) override {
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

} // namespace toyc::ast