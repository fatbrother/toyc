#include "ast/structure.hpp"

#include <iostream>

using namespace toyc::ast;

CodegenResult NStructType::codegen(ASTContext &context) {
    if (members == nullptr) {
        return CodegenResult("Struct has no members: " + name);
    }

    std::vector<llvm::Type *> memberTypes;
    for (auto currentMember = members; currentMember != nullptr; currentMember = currentMember->next) {
        llvm::Type *memberType = currentMember->type->getLLVMType(context.llvmContext);
        if (memberType == nullptr) {
            return CodegenResult("Member type is null in struct: " + name);
        }
        memberTypes.push_back(memberType);
    }

    llvm::StructType *structType = llvm::StructType::create(context.llvmContext, memberTypes, name);
    if (structType == nullptr) {
        return CodegenResult("Failed to create struct type: " + name);
    }

    return CodegenResult(nullptr);
}