#include "ast/structure.hpp"

#include <iostream>

using namespace toyc::ast;

CodegenResult NStructType::codegen(ASTContext &context) {
    if (members == nullptr) {
        return CodegenResult("Struct has no members: " + name);
    }

    std::vector<llvm::Type *> memberTypes;
    for (auto currentMember = members; currentMember != nullptr; currentMember = currentMember->next) {
        CodegenResult typeResult = currentMember->type->getLLVMType(context);
        if (false == typeResult.isSuccess() || nullptr == typeResult.getLLVMType()) {
            return typeResult << CodegenResult("Failed to get LLVM type for member in struct: " + name);
        }
        llvm::Type *memberType = typeResult.getLLVMType();
        if (memberType == nullptr) {
            return CodegenResult("Member type is null in struct: " + name);
        }
        memberTypes.push_back(memberType);
    }

    llvmStructType = llvm::StructType::create(context.llvmContext, memberTypes, name);
    if (llvmStructType == nullptr) {
        return CodegenResult("Failed to create struct type: " + name);
    }

    return CodegenResult();
}