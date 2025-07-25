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
    VAR_TYPE_PTR,
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

enum UnaryOperator {
    L_INC,
    R_INC,
    L_DEC,
    R_DEC,
    NOT,
    ADDR,
    DEREF,
};

#define SAFE_DELETE(ptr) \
    if (ptr) {           \
        delete ptr;      \
        ptr = nullptr;   \
    }

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
    NType(VarType type, NType *pointTo)
        : type(type), pointTo(pointTo) {}
    virtual std::string getType() const override { return "Type"; }
    virtual VarType getVarType() const { return type; }
    virtual llvm::Type *getLLVMType(llvm::LLVMContext &context) const;

private:
    VarType type;
    std::string name;
    NType *pointTo = nullptr;
};

static llvm::Value *typeCast(llvm::Value *value, const VarType toType, llvm::LLVMContext &context, llvm::IRBuilder<> &builder) {
    if (nullptr == value) {
        throw std::runtime_error("Invalid type cast: value or type is null");
    }

    switch (toType) {
        case VAR_TYPE_INT:
            return builder.CreateIntCast(value, llvm::Type::getInt32Ty(context), false);
        case VAR_TYPE_SHORT:
            return builder.CreateIntCast(value, llvm::Type::getInt16Ty(context), false);
        case VAR_TYPE_LONG:
            return builder.CreateIntCast(value, llvm::Type::getInt64Ty(context), false);
        case VAR_TYPE_CHAR:
            return builder.CreateIntCast(value, llvm::Type::getInt8Ty(context), false);
        case VAR_TYPE_BOOL:
            return builder.CreateIntCast(value, llvm::Type::getInt1Ty(context), false);
        case VAR_TYPE_FLOAT:
            return builder.CreateFPCast(value, llvm::Type::getFloatTy(context));
        case VAR_TYPE_DOUBLE:
            return builder.CreateFPCast(value, llvm::Type::getDoubleTy(context));
        default:
            throw std::runtime_error("Unsupported type cast");
    }

    return value;
}

class NBlock;
class NParentStatement;

class NExpression : public BasicNode {
public:
    virtual llvm::Value *codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) = 0;
    virtual llvm::AllocaInst *allocgen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder, NParentStatement *parent) {
        throw std::runtime_error("Expression is not a valid left value");
    };
    virtual std::string getType() const override { return "Expression"; }
};

class NExternalDeclaration : public BasicNode {
public:
    virtual ~NExternalDeclaration() {
        SAFE_DELETE(next);
    }
    virtual void codegen(llvm::LLVMContext &context, llvm::Module &module, llvm::IRBuilder<> &builder) = 0;

public:
    NExternalDeclaration *next = nullptr;
};


} // namespace toyc::ast
