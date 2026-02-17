#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>

#include <algorithm>
#include <iostream>
#include <map>
#include <stack>
#include <string>
#include <unordered_set>

#include "ast/codegen_result.hpp"
#include "ast/type.hpp"

namespace toyc::ast {

class NFunctionDefinition;

template <typename T>
class ScopeTable {
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
    void insert(const std::string &name, T obj) { variables[name] = obj; }

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

    llvm::BasicBlock *getContinueTarget() const { return continueTarget; }
    llvm::BasicBlock *getBreakTarget() const { return breakTarget; }

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
    NSwitchContext(llvm::BasicBlock *breakTarget) : NJumpContext(nullptr, breakTarget) {}

    virtual bool supportsContinue() const override { return false; }
    virtual bool supportsBreak() const override { return true; }
    virtual std::string getContextName() const override { return "switch"; }
};

struct ASTContext {
    llvm::LLVMContext llvmContext;
    llvm::Module module;
    llvm::IRBuilder<> builder;
    NFunctionDefinition *currentFunction = nullptr;
    ScopeTable<std::pair<llvm::AllocaInst *, TypeIdx>> *variableTable = nullptr;

    std::map<std::string, NFunctionDefinition *> functionDefinitions;
    bool isInitializingFunction = false;

    std::unique_ptr<TypeManager> typeManager;
    TypeManager &getTypeManager() { return *typeManager; }

    // Jump context stack for break/continue statements
    // Used by loops (for/while/do-while) and switch statements
    std::stack<std::shared_ptr<NJumpContext>> jumpContextStack;

    // Label management for goto statements
    std::map<std::string, llvm::BasicBlock *> labels;
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
    void registerLabel(const std::string &name, llvm::BasicBlock *block);
    llvm::BasicBlock *getLabel(const std::string &name);
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
    virtual ~NExternalDeclaration() { SAFE_DELETE(next); }

    virtual StmtCodegenResult codegen(ASTContext &context) = 0;

public:
    NExternalDeclaration *next = nullptr;
};

}  // namespace toyc::ast
