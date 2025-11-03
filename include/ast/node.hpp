#pragma once

#include <string>
#include <stack>
#include <map>
#include <unordered_set>
#include <algorithm>
#include <iostream>
#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Instructions.h>

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
    CodegenResult(llvm::Type *type) : errorMessage(""), llvmType(type), value(nullptr) {}
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
    llvm::Type *getLLVMType() const { return llvmType; }
    NTypePtr getType() const { return type; }

private:
    std::string errorMessage;
    llvm::Value *value = nullptr;
    llvm::Type *llvmType = nullptr;
    NTypePtr type = nullptr;
};

template<typename T> class ScopeTable {
public:
    ScopeTable() = default;
    ScopeTable(ScopeTable<T> *parent) : parent(parent) {}

    /**
     * Looks up a object in the current scope and parent scopes.
     * @return A pair of (isFound, object).
     */
    std::pair<bool, T> lookup(const std::string &name, bool deepSearch = true) {
        auto it = variables.find(name);
        if (it != variables.end()) {
            return std::make_pair(true, it->second);
        }

        if (false == deepSearch || nullptr == parent) {
            return std::make_pair(false, T());
        }

        return parent->lookup(name);
    }

    /**
     * Inserts a new object into the current scope.
     */
    void insert(const std::string &name, T obj) {
        variables[name] = obj;
    }

private:
    std::map<std::string, T> variables;
    ScopeTable<T> *parent = nullptr;

friend class ASTContext;
};

// Jump context for break/continue statements
// Used by loops (for/while/do-while) and switch statements
class NJumpContext {
public:
    NJumpContext(llvm::BasicBlock *continueTarget, llvm::BasicBlock *breakTarget)
        : continueTarget(continueTarget), breakTarget(breakTarget) {}

    llvm::BasicBlock* getContinueTarget() const { return continueTarget; }
    llvm::BasicBlock* getBreakTarget() const { return breakTarget; }

    virtual bool supportsContinue() const { return true; }
    virtual bool supportsBreak() const { return true; }
    virtual std::string getContextName() const { return "loop"; }

protected:
    llvm::BasicBlock *continueTarget;
    llvm::BasicBlock *breakTarget;
};

// Switch context - only supports break, not continue
class NSwitchContext : public NJumpContext {
public:
    NSwitchContext(llvm::BasicBlock *breakTarget)
        : NJumpContext(nullptr, breakTarget) {}

    virtual bool supportsContinue() const override { return false; }
    virtual bool supportsBreak() const override { return true; }
    virtual std::string getContextName() const override { return "switch"; }
};

struct ASTContext {
    llvm::LLVMContext llvmContext;
    llvm::Module module;
    llvm::IRBuilder<> builder;
    NFunctionDefinition *currentFunction = nullptr;
    ScopeTable<std::pair<llvm::AllocaInst *, NTypePtr>> *variableTable = nullptr;
    ScopeTable<NTypePtr> *typeTable = nullptr;

    std::map<std::string, NFunctionDefinition *> functionDefinitions;
    bool isInitializingFunction = false;

    // Jump context stack for break/continue statements
    // Used by loops (for/while/do-while) and switch statements
    std::stack<std::shared_ptr<NJumpContext>> jumpContextStack;

    // Label management for goto statements
    std::map<std::string, llvm::BasicBlock*> labels;
    std::unordered_set<std::string> pendingGotos;

    // Switch statement tracking (for case/default statements)
    llvm::SwitchInst *currentSwitch = nullptr;
    llvm::BasicBlock *currentSwitchAfter = nullptr;
    llvm::BasicBlock *currentSwitchDefault = nullptr;
    bool switchHasDefault = false;

    ASTContext() : module("toyc", llvmContext), builder(llvmContext) {
        pushScope();
    }

    // Jump context management helpers
    void pushJumpContext(std::shared_ptr<NJumpContext> ctx) {
        jumpContextStack.push(ctx);
    }

    void popJumpContext() {
        if (!jumpContextStack.empty()) {
            jumpContextStack.pop();
        }
    }

    std::shared_ptr<NJumpContext> getCurrentJumpContext() const {
        return jumpContextStack.empty() ? nullptr : jumpContextStack.top();
    }

    // Label management for goto
    void registerLabel(const std::string& name, llvm::BasicBlock* block) {
        labels[name] = block;
    }

    llvm::BasicBlock* getLabel(const std::string& name) {
        auto it = labels.find(name);
        return (it != labels.end()) ? it->second : nullptr;
    }

    void clearLabels() {
        labels.clear();
        pendingGotos.clear();
    }

    void pushScope() {
        variableTable = new ScopeTable<std::pair<llvm::AllocaInst *, NTypePtr>>(variableTable);
        typeTable = new ScopeTable<NTypePtr>(typeTable);
    }

    void popScope() {
        if (variableTable) {
            ScopeTable<std::pair<llvm::AllocaInst *, NTypePtr>> *parent = variableTable->parent;
            delete variableTable;
            variableTable = parent;
        }
        if (typeTable) {
            ScopeTable<NTypePtr> *parent = typeTable->parent;
            delete typeTable;
            typeTable = parent;
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
    virtual CodegenResult codegen(ASTContext &context) {
        return CodegenResult("Codegen not implemented for this node type: " + getType());
    }
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
    virtual CodegenResult getLLVMType(ASTContext &context) const;

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

static std::string varTypeToString(VarType type) {
    switch (type) {
        case VAR_TYPE_VOID: return "void";
        case VAR_TYPE_CHAR: return "char";
        case VAR_TYPE_BOOL: return "bool";
        case VAR_TYPE_SHORT: return "short";
        case VAR_TYPE_INT: return "int";
        case VAR_TYPE_LONG: return "long";
        case VAR_TYPE_FLOAT: return "float";
        case VAR_TYPE_DOUBLE: return "double";
        case VAR_TYPE_PTR: return "pointer";
        case VAR_TYPE_STRUCT: return "struct";
        case VAR_TYPE_DEFINED: return "defined";
        default: return "unknown";
    }
}

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

static CodegenResult typeCast(llvm::Value *value, const NTypePtr fromType, const NTypePtr toType, ASTContext &context) {
    if (nullptr == value || nullptr == fromType || nullptr == toType) {
        return CodegenResult("Type cast failed due to null value or type");
    }

    VarType from = fromType->type;
    VarType to = toType->type;

    if (from == to) {
        return value;
    }

    if (from == VAR_TYPE_BOOL) {
        return castFromBool(value, to, context.llvmContext, context.builder);
    }

    if (to == VAR_TYPE_BOOL) {
        return castToBool(value, from, context.llvmContext, context.builder);
    }

    switch (to) {
        case VAR_TYPE_CHAR:
            if (isFloatingPointType(from)) {
                return context.builder.CreateFPToSI(value, llvm::Type::getInt8Ty(context.llvmContext), "to_char");
            }
            return context.builder.CreateIntCast(value, llvm::Type::getInt8Ty(context.llvmContext), true, "to_char");

        case VAR_TYPE_SHORT:
            if (isFloatingPointType(from)) {
                return context.builder.CreateFPToSI(value, llvm::Type::getInt16Ty(context.llvmContext), "to_short");
            }
            return context.builder.CreateIntCast(value, llvm::Type::getInt16Ty(context.llvmContext), true, "to_short");

        case VAR_TYPE_INT:
            if (isFloatingPointType(from)) {
                return context.builder.CreateFPToSI(value, llvm::Type::getInt32Ty(context.llvmContext), "to_int");
            }
            return context.builder.CreateIntCast(value, llvm::Type::getInt32Ty(context.llvmContext), true, "to_int");

        case VAR_TYPE_LONG:
            if (isFloatingPointType(from)) {
                return context.builder.CreateFPToSI(value, llvm::Type::getInt64Ty(context.llvmContext), "to_long");
            }
            return context.builder.CreateIntCast(value, llvm::Type::getInt64Ty(context.llvmContext), true, "to_long");

        case VAR_TYPE_FLOAT:
            if (isIntegerType(from)) {
                return context.builder.CreateSIToFP(value, llvm::Type::getFloatTy(context.llvmContext), "to_float");
            }
            return context.builder.CreateFPCast(value, llvm::Type::getFloatTy(context.llvmContext), "to_float");

        case VAR_TYPE_DOUBLE:
            if (isIntegerType(from)) {
                return context.builder.CreateSIToFP(value, llvm::Type::getDoubleTy(context.llvmContext), "to_double");
            }
            return context.builder.CreateFPCast(value, llvm::Type::getDoubleTy(context.llvmContext), "to_double");

        case VAR_TYPE_PTR:
            if (isIntegerType(from)) {
                CodegenResult ptrTypeResult = toType->pointTo->getLLVMType(context);
                if (false == ptrTypeResult.isSuccess() || nullptr == ptrTypeResult.getLLVMType()) {
                    return ptrTypeResult << CodegenResult("Failed to get LLVM type for pointer type cast");
                }
                return context.builder.CreateIntToPtr(value, ptrTypeResult.getLLVMType(), "to_ptr");
            }
            return context.builder.CreateBitCast(value, toType->getLLVMType(context).getLLVMType(), "to_ptr");

        default:
            return CodegenResult("Unsupported type cast from " + varTypeToString(from) + " to " + varTypeToString(to));
    }
}

static CodegenResult typeCast(llvm::Value *value, const NTypePtr fromType, VarType toType, ASTContext &context) {
    NTypePtr toNType = std::make_shared<NType>(toType);
    return typeCast(value, fromType, toNType, context);
}

} // namespace toyc::ast
