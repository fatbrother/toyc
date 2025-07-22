#pragma once

#include <string>
#include <stack>
#include <map>
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
    ADDR,
    DEREF,
    PLUS,
    MINUS,
    LOG_NOT,
    BIT_NOT
};

#define SAFE_DELETE(ptr) \
    if (ptr) {           \
        delete ptr;      \
        ptr = nullptr;   \
    }

class NBlock;
class NParentStatement;
class NFunctionDefinition;

struct VariableTable {
    std::map<std::string, llvm::AllocaInst *> variables;
    VariableTable *parent = nullptr;

    llvm::AllocaInst *lookupVariable(const std::string &name) {
        auto it = variables.find(name);
        if (it != variables.end()) {
            return it->second;
        }
        return nullptr == parent ? nullptr : parent->lookupVariable(name);
    }
    void insertVariable(const std::string &name, llvm::AllocaInst *allocaInst) {
        variables[name] = allocaInst;
    }
};

struct ASTContext {
    llvm::LLVMContext llvmContext;
    llvm::Module module;
    llvm::IRBuilder<> builder;
    NFunctionDefinition *currentFunction = nullptr;
    VariableTable *variableTable = nullptr;
    bool isInitializingFunction = false;

    ASTContext() : module("toyc", llvmContext), builder(llvmContext) {}

    void pushVariableTable() {
        VariableTable *newTable = new VariableTable();
        newTable->parent = variableTable;
        variableTable = newTable;
    }

    void popVariableTable() {
        if (variableTable) {
            VariableTable *oldTable = variableTable;
            variableTable = variableTable->parent;
            delete oldTable;
        }
    }
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

class NExpression : public BasicNode {
public:
    virtual llvm::Value *codegen(ASTContext &context) = 0;
    virtual llvm::AllocaInst *allocgen(ASTContext &context) {
        throw std::runtime_error("Expression is not a valid left value");
    };
    virtual std::string getType() const override { return "Expression"; }
};

class NExternalDeclaration : public BasicNode {
public:
    virtual ~NExternalDeclaration() {
        SAFE_DELETE(next);
    }
    virtual void codegen(ASTContext &context) = 0;

public:
    NExternalDeclaration *next = nullptr;
};

class NParameter : public BasicNode {
public:
    NParameter(NType *type, const std::string &name, bool isVarArg = false)
        : type(type), name(name), isVarArg(isVarArg) {}
    ~NParameter() {
        SAFE_DELETE(type);
        SAFE_DELETE(next);
    }
    virtual std::string getType() const override { return "Parameter"; }
    llvm::Type *getLLVMType(llvm::LLVMContext &context) const {
        return type->getLLVMType(context);
    }
    std::string getName() const { return name; }

public:
    NParameter *next = nullptr;
    bool isVarArg = false;

private:
    NType *type;
    std::string name;
};


class NFunctionDefinition : public NExternalDeclaration {
public:
    NFunctionDefinition(NType *returnType, const std::string &name, NParameter *params, NBlock *body)
        : returnType(returnType), name(name), params(params), body(body) {}
    ~NFunctionDefinition();
    virtual void codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "FunctionDefinition"; }
    llvm::Function *getFunction() const { return llvmFunction; }
    NType *getReturnType() const { return returnType; }

private:
    llvm::Function *llvmFunction = nullptr;
    std::string name;
    NType *returnType;
    NParameter *params;
    NBlock *body;
};

} // namespace toyc::ast
