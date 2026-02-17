#include "ast/node.hpp"

namespace toyc::ast {

ASTContext::ASTContext()
    : module("toyc", llvmContext),
      builder(llvmContext),
      typeManager(std::make_unique<TypeManager>(llvmContext, module)) {
    pushScope();
}

void ASTContext::pushJumpContext(std::shared_ptr<NJumpContext> ctx) {
    jumpContextStack.push(ctx);
}

void ASTContext::popJumpContext() {
    if (!jumpContextStack.empty()) {
        jumpContextStack.pop();
    }
}

std::shared_ptr<NJumpContext> ASTContext::getCurrentJumpContext() const {
    return jumpContextStack.empty() ? nullptr : jumpContextStack.top();
}

void ASTContext::registerLabel(const std::string& name, llvm::BasicBlock* block) {
    labels[name] = block;
}

llvm::BasicBlock* ASTContext::getLabel(const std::string& name) {
    auto it = labels.find(name);
    return (it != labels.end()) ? it->second : nullptr;
}

void ASTContext::clearLabels() {
    labels.clear();
    pendingGotos.clear();
}

void ASTContext::pushScope() {
    variableTable = new ScopeTable<std::pair<llvm::AllocaInst*, TypeIdx>>(variableTable);
}

void ASTContext::popScope() {
    if (variableTable) {
        ScopeTable<std::pair<llvm::AllocaInst*, TypeIdx>>* parent = variableTable->parent;
        delete variableTable;
        variableTable = parent;
    }
}

}  // namespace toyc::ast
