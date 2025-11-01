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

class CodegenResult {
public:
    CodegenResult() : errorMessage(""), value(nullptr) {}
    CodegenResult(llvm::Value *val) : errorMessage(""), value(val), type(nullptr) {}
    CodegenResult(llvm::Value *val, NTypePtr type) : errorMessage(""), value(val), type(type) {}
    CodegenResult(const std::string &errMsg) : errorMessage(errMsg), value(nullptr) {}

    CodegenResult& operator<<(const CodegenResult &other) {
        if (!other.isSuccess()) {
            if (false == this->isSuccess()) {
                if (!this->errorMessage.empty()) {
                    this->errorMessage += "\n";
                }

                this->errorMessage += other.errorMessage;
            }
        }
        return *this;
    }

    bool isSuccess() const { return "" == errorMessage; }
    std::string getErrorMessage() const { return errorMessage; }
    llvm::Value *getValue() const { return value; }
    NTypePtr getType() const { return type; }

private:
    std::string errorMessage;
    llvm::Value *value = nullptr;
    NTypePtr type = nullptr;
};

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
    virtual CodegenResult codegen(ASTContext &context) = 0;
};

class NType {
public:
    NType(VarType type, const std::string &name = "")
        : type(type), name(name) {}
    NType(VarType type, NTypePtr pointTo, int pointerLevel = 0)
        : type(type), pointTo(pointTo), pointerLevel(pointerLevel) {}
    NType(VarType type, NType *pointTo, int pointerLevel = 0)
        : type(type), pointTo(pointTo), pointerLevel(pointerLevel) {}

    virtual std::string getType() const { return "Type"; }
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

static CodegenResult castFromBool(llvm::Value *value, VarType toType,
                                 llvm::LLVMContext &context, llvm::IRBuilder<> &builder) {
    llvm::Value *result = nullptr;

    switch (toType) {
        case VAR_TYPE_CHAR:
            result = builder.CreateZExt(value, llvm::Type::getInt8Ty(context), "bool_to_char");
            break;
        case VAR_TYPE_SHORT:
            result = builder.CreateZExt(value, llvm::Type::getInt16Ty(context), "bool_to_short");
            break;
        case VAR_TYPE_INT:
            result = builder.CreateZExt(value, llvm::Type::getInt32Ty(context), "bool_to_int");
            break;
        case VAR_TYPE_LONG:
            result = builder.CreateZExt(value, llvm::Type::getInt64Ty(context), "bool_to_long");
            break;
        case VAR_TYPE_FLOAT:
            result = builder.CreateUIToFP(value, llvm::Type::getFloatTy(context), "bool_to_float");
            break;
        case VAR_TYPE_DOUBLE:
            result = builder.CreateUIToFP(value, llvm::Type::getDoubleTy(context), "bool_to_double");
            break;
        default:
            return CodegenResult("Unsupported cast from bool to " + std::to_string(toType));
    }

    return CodegenResult(result);
}

static llvm::Value *castToBool(llvm::Value *value, VarType fromType,
                               llvm::LLVMContext &context, llvm::IRBuilder<> &builder) {
    if (isFloatingPointType(fromType)) {
        return builder.CreateFCmpONE(value, llvm::ConstantFP::get(value->getType(), 0.0), "to_bool");
    } else {
        return builder.CreateICmpNE(value, llvm::ConstantInt::get(value->getType(), 0), "to_bool");
    }
}

static CodegenResult typeCast(llvm::Value *value, const NTypePtr fromType, const NTypePtr toType, llvm::LLVMContext &context, llvm::IRBuilder<> &builder) {
    if (nullptr == value || nullptr == fromType || nullptr == toType) {
        return CodegenResult("Type cast failed due to null value or type");
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
            return CodegenResult("Unsupported type cast from " + std::to_string(from) + " to " + std::to_string(to));
    }
}

static CodegenResult typeCast(llvm::Value *value, const NTypePtr fromType, VarType toType, llvm::LLVMContext &context, llvm::IRBuilder<> &builder) {
    NTypePtr toNType = std::make_shared<NType>(toType);
    return typeCast(value, fromType, toNType, context, builder);
}

} // namespace toyc::ast
