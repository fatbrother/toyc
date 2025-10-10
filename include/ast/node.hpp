#pragma once

#include <string>
#include <stack>
#include <map>
#include <iostream>
#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>

namespace toyc::ast {

enum VarType {
    VAR_TYPE_VOID = 0,
    VAR_TYPE_DEFINED = 1,
    VAR_TYPE_CHAR = 2,
    VAR_TYPE_BOOL = 3,
    VAR_TYPE_SHORT = 4,
    VAR_TYPE_INT = 5,
    VAR_TYPE_LONG = 6,
    VAR_TYPE_FLOAT = 7,
    VAR_TYPE_DOUBLE = 8,
    VAR_TYPE_PTR = 9,
    VAR_TYPE_STRUCT = 10,
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
    LT,
    GT,
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

class NFunctionDefinition;
class NType;
typedef std::shared_ptr<NType> NTypePtr;

struct VariableTable {
    std::map<std::string, std::pair<llvm::AllocaInst *, NTypePtr>> variables;
    VariableTable *parent = nullptr;

    /**
     * Looks up a variable in the current scope and parent scopes.
     * @return A pair containing the AllocaInst and NTypePtr if found, otherwise {nullptr, nullptr}.
     */
    std::pair<llvm::AllocaInst *, NTypePtr> lookupVariable(const std::string &name, bool searchParent = true) {
        auto it = variables.find(name);
        if (it != variables.end()) {
            return it->second;
        }

        if (false == searchParent || nullptr == parent) {
            return std::make_pair(nullptr, nullptr);
        }

        return parent->lookupVariable(name);
    }

    /**
     * Inserts a new variable into the current scope.
     */
    void insertVariable(const std::string &name, llvm::AllocaInst *allocaInst, NTypePtr type) {
        variables[name] = std::make_pair(allocaInst, type);
    }
};

struct ASTContext {
    llvm::LLVMContext llvmContext;
    llvm::Module module;
    llvm::IRBuilder<> builder;
    NFunctionDefinition *currentFunction = nullptr;
    VariableTable *variableTable = nullptr;
    std::map<std::string, NFunctionDefinition *> functionDefinitions;
    bool isInitializingFunction = false;

    ASTContext() : module("toyc", llvmContext), builder(llvmContext) {}

    void pushScope() {
        VariableTable *newTable = new VariableTable();
        newTable->parent = variableTable;
        variableTable = newTable;
    }

    void popScope() {
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
    NType(VarType type, NTypePtr pointTo, int pointerLevel = 0)
        : type(type), pointTo(pointTo), pointerLevel(pointerLevel) {}
    NType(VarType type, NType *pointTo, int pointerLevel = 0)
        : type(type), pointTo(pointTo), pointerLevel(pointerLevel) {}

    virtual std::string getType() const override { return "Type"; }
    virtual llvm::Type *getLLVMType(llvm::LLVMContext &context) const;

    std::string name;
    NTypePtr pointTo = nullptr;
    int pointerLevel = 0;
    VarType type;
};

class NExternalDeclaration : public BasicNode {
public:
    virtual ~NExternalDeclaration() {
        SAFE_DELETE(next);
    }
    virtual llvm::Value *codegen(ASTContext &context) = 0;

public:
    NExternalDeclaration *next = nullptr;
};

static bool isFloatingPointType(VarType type) {
    return type == VAR_TYPE_FLOAT || type == VAR_TYPE_DOUBLE;
}

static bool isIntegerType(VarType type) {
    return type == VAR_TYPE_CHAR || type == VAR_TYPE_BOOL ||
           type == VAR_TYPE_SHORT || type == VAR_TYPE_INT || type == VAR_TYPE_LONG;
}

static llvm::Value *castFromBool(llvm::Value *value, VarType toType,
                                 llvm::LLVMContext &context, llvm::IRBuilder<> &builder) {
    switch (toType) {
        case VAR_TYPE_CHAR:
            return builder.CreateZExt(value, llvm::Type::getInt8Ty(context), "bool_to_char");
        case VAR_TYPE_SHORT:
            return builder.CreateZExt(value, llvm::Type::getInt16Ty(context), "bool_to_short");
        case VAR_TYPE_INT:
            return builder.CreateZExt(value, llvm::Type::getInt32Ty(context), "bool_to_int");
        case VAR_TYPE_LONG:
            return builder.CreateZExt(value, llvm::Type::getInt64Ty(context), "bool_to_long");
        case VAR_TYPE_FLOAT:
            return builder.CreateUIToFP(value, llvm::Type::getFloatTy(context), "bool_to_float");
        case VAR_TYPE_DOUBLE:
            return builder.CreateUIToFP(value, llvm::Type::getDoubleTy(context), "bool_to_double");
        default:
            throw std::runtime_error("Unsupported cast from bool to type: " + std::to_string(toType));
    }
}

static llvm::Value *castToBool(llvm::Value *value, VarType fromType,
                               llvm::LLVMContext &context, llvm::IRBuilder<> &builder) {
    if (isFloatingPointType(fromType)) {
        return builder.CreateFCmpONE(value, llvm::ConstantFP::get(value->getType(), 0.0), "to_bool");
    } else {
        return builder.CreateICmpNE(value, llvm::ConstantInt::get(value->getType(), 0), "to_bool");
    }
}

static llvm::Value *typeCast(llvm::Value *value, const NTypePtr fromType, const NTypePtr toType, llvm::LLVMContext &context, llvm::IRBuilder<> &builder) {
    if (nullptr == value || nullptr == fromType || nullptr == toType) {
        throw std::runtime_error("Invalid type cast: value or type is null");
    }

    VarType from = fromType->type;
    VarType to = toType->type;

    if (from == to) {
        return value;
    }

    if (from == VAR_TYPE_BOOL) {
        return castFromBool(value, to, context, builder);
    }

    if (to == VAR_TYPE_BOOL) {
        return castToBool(value, from, context, builder);
    }

    switch (to) {
        case VAR_TYPE_CHAR:
            if (isFloatingPointType(from)) {
                return builder.CreateFPToSI(value, llvm::Type::getInt8Ty(context), "to_char");
            }
            return builder.CreateIntCast(value, llvm::Type::getInt8Ty(context), true, "to_char");

        case VAR_TYPE_SHORT:
            if (isFloatingPointType(from)) {
                return builder.CreateFPToSI(value, llvm::Type::getInt16Ty(context), "to_short");
            }
            return builder.CreateIntCast(value, llvm::Type::getInt16Ty(context), true, "to_short");

        case VAR_TYPE_INT:
            if (isFloatingPointType(from)) {
                return builder.CreateFPToSI(value, llvm::Type::getInt32Ty(context), "to_int");
            }
            return builder.CreateIntCast(value, llvm::Type::getInt32Ty(context), true, "to_int");

        case VAR_TYPE_LONG:
            if (isFloatingPointType(from)) {
                return builder.CreateFPToSI(value, llvm::Type::getInt64Ty(context), "to_long");
            }
            return builder.CreateIntCast(value, llvm::Type::getInt64Ty(context), true, "to_long");

        case VAR_TYPE_FLOAT:
            if (isIntegerType(from)) {
                return builder.CreateSIToFP(value, llvm::Type::getFloatTy(context), "to_float");
            }
            return builder.CreateFPCast(value, llvm::Type::getFloatTy(context), "to_float");

        case VAR_TYPE_DOUBLE:
            if (isIntegerType(from)) {
                return builder.CreateSIToFP(value, llvm::Type::getDoubleTy(context), "to_double");
            }
            return builder.CreateFPCast(value, llvm::Type::getDoubleTy(context), "to_double");

        case VAR_TYPE_PTR:
            if (isIntegerType(from)) {
                return builder.CreateIntToPtr(value, toType->pointTo->getLLVMType(context), "to_ptr");
            }
            return builder.CreateBitCast(value, toType->pointTo->getLLVMType(context), "to_ptr");

        default:
            throw std::runtime_error("Unsupported type cast to type: " + std::to_string(to));
    }
}

static llvm::Value *typeCast(llvm::Value *value, const NTypePtr fromType, VarType toType, llvm::LLVMContext &context, llvm::IRBuilder<> &builder) {
    NTypePtr toNType = std::make_shared<NType>(toType);
    return typeCast(value, fromType, toNType, context, builder);
}

} // namespace toyc::ast
