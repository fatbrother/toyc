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

#include "ast/type.hpp"

namespace toyc::ast {

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

template<typename T>
class CodegenResult {
public:
    CodegenResult() : errorMessage("") {}
    CodegenResult(const T& data) : data(data), errorMessage("") {}
    CodegenResult(const std::string& errMsg) : errorMessage(errMsg) {}
    CodegenResult(const char *errMsg) : errorMessage(std::string(errMsg)) {}

    template<typename... Args>
    CodegenResult(Args... args) : data(args...), errorMessage("") {}

    template<typename U>
    CodegenResult& operator<<(const CodegenResult<U>& other) {
        if (!other.isSuccess()) {
            if (errorMessage.empty()) {
                errorMessage = other.getErrorMessage();
            } else {
                errorMessage = errorMessage + "\n" + other.getErrorMessage();
            }
        }
        return *this;
    }

    bool isSuccess() const {
        if constexpr (std::is_same_v<T, void>) {
            return errorMessage.empty();
        } else {
            return isSuccessImpl(0);
        }
    }

private:
    template<typename U = T>
    auto isSuccessImpl(int) const -> decltype(std::declval<U>().isValid(), bool()) {
        return errorMessage.empty() && data.isValid();
    }

    template<typename U = T>
    bool isSuccessImpl(long) const {
        return errorMessage.empty();
    }

public:

    std::string getErrorMessage() const { return errorMessage; }

    T getData() const { return data; }

    template<typename U = T>
    auto getValue() const -> decltype(std::declval<U>().value) {
        return data.value;
    }

    template<typename U = T>
    auto getType() const -> decltype(std::declval<U>().type) {
        return data.type;
    }

    template<typename U = T>
    auto getAllocaInst() const -> decltype(std::declval<U>().allocInst) {
        return data.allocInst;
    }

private:

    T data;
    std::string errorMessage;
};

template<>
class CodegenResult<void> {
public:
    CodegenResult() : errorMessage("") {}
    CodegenResult(const std::string& errMsg) : errorMessage(errMsg) {}

    template<typename U>
    CodegenResult& operator<<(const CodegenResult<U>& other) {
        if (!other.isSuccess()) {
            if (errorMessage.empty()) {
                errorMessage = other.getErrorMessage();
            } else {
                errorMessage = errorMessage + "\n" + other.getErrorMessage();
            }
        }
        return *this;
    }

    bool isSuccess() const { return errorMessage.empty(); }
    std::string getErrorMessage() const { return errorMessage; }

private:
    std::string errorMessage;
};

/**
 * Value type for Expression code generation results.
 */
struct ExprValue {
    llvm::Value* value = nullptr;
    llvm::Type* type = nullptr;

    ExprValue() = default;
    ExprValue(llvm::Value* v, llvm::Type* t) : value(v), type(t) {}
    bool isValid() const { return value != nullptr && type != nullptr; }
};

struct AllocValue {
    llvm::Value* allocInst = nullptr;
    llvm::Type* type = nullptr;

    AllocValue() = default;
    AllocValue(llvm::Value* a, llvm::Type* t) : allocInst(a), type(t) {}
    bool isValid() const { return allocInst != nullptr && type != nullptr; }
};

// Type aliases for common use cases
using ExprCodegenResult = CodegenResult<ExprValue>;
using StmtCodegenResult = CodegenResult<void>;
using AllocCodegenResult = CodegenResult<AllocValue>;

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
    ScopeTable<std::pair<llvm::AllocaInst *, llvm::Type*>> *variableTable = nullptr;

    std::map<std::string, NFunctionDefinition *> functionDefinitions;
    bool isInitializingFunction = false;

    std::unique_ptr<TypeManager> typeManager;

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

    ASTContext();

    // Jump context management helpers
    void pushJumpContext(std::shared_ptr<NJumpContext> ctx);
    void popJumpContext();
    std::shared_ptr<NJumpContext> getCurrentJumpContext() const;

    // Label management for goto
    void registerLabel(const std::string& name, llvm::BasicBlock* block);
    llvm::BasicBlock* getLabel(const std::string& name);
    void clearLabels();

    void pushScope();
    void popScope();
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

class NExternalDeclaration : public BasicNode {
public:
    virtual ~NExternalDeclaration() {
        SAFE_DELETE(next);
    }

    virtual StmtCodegenResult codegen(ASTContext &context) = 0;

public:
    NExternalDeclaration *next = nullptr;
};

CodegenResult<ExprValue> typeCast(llvm::Value *value, llvm::Type* fromType,
                                  llvm::Type* toType, toyc::ast::ASTContext &context);

} // namespace toyc::ast
