#pragma once

#include <string>
#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>

namespace toyc::ast {

enum VarType {
    VAR_TYPE_INT,
    VAR_TYPE_SHORT,
    VAR_TYPE_LONG,
    VAR_TYPE_CHAR,
    VAR_TYPE_BOOL,
    VAR_TYPE_FLOAT,
    VAR_TYPE_DOUBLE,
    VAR_TYPE_VOID,
    VAR_TYPE_DEFINED,
};

enum BineryOperator {
    AND,
    OR,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    LEFT,
    RIGHT,
    EQ,
    NE,
    LE,
    GE,
    L,
    G,
    BIT_AND,
    BIT_OR,
    XOR
};

class BasicNode {
public:
    virtual ~BasicNode() = default;
    BasicNode() = default;
    BasicNode(const BasicNode &) = default;
    BasicNode(BasicNode &&) = default;
    BasicNode &operator=(const BasicNode &) = default;
    virtual std::string getType() const = 0;
};

class NType : public BasicNode {
public:
    NType(VarType type, const std::string &name = "")
        : type(type), name(name) {}
    virtual std::string getType() const override { return "Type"; }
    virtual llvm::Type *getLLVMType(llvm::LLVMContext &context) const;

private:
    VarType type;
    std::string name;
};

class NBlock;

class NStatement : public BasicNode {
public:
    virtual void codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) = 0;
    virtual std::string getType() const override { return "Statement"; }
    void setParent(NBlock *parent) { this->parent = parent; }

public:
    NBlock *parent = nullptr;
    NStatement *next = nullptr;
};

class NExpression : public BasicNode {
public:
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NBlock *parent) = 0;
    virtual std::string getType() const override { return "Expression"; }
};

class NExternalDeclaration : public BasicNode {
public:
    virtual void codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) = 0;

public:
    NExternalDeclaration *next = nullptr;
};


} // namespace toyc::ast
