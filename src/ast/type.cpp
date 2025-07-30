#include "ast/node.hpp"

#include <iostream>

using namespace toyc::ast;

llvm::Type *NType::getLLVMType(llvm::LLVMContext &context) const {
    llvm::Type *llvmType = nullptr;

    switch (type) {
        case VAR_TYPE_VOID:
            llvmType = llvm::Type::getVoidTy(context);
            break;
        case VAR_TYPE_CHAR:
            llvmType = llvm::Type::getInt8Ty(context);
            break;
        case VAR_TYPE_BOOL:
            llvmType = llvm::Type::getInt1Ty(context);
            break;
        case VAR_TYPE_SHORT:
            llvmType = llvm::Type::getInt16Ty(context);
            break;
        case VAR_TYPE_INT:
            llvmType = llvm::Type::getInt32Ty(context);
            break;
        case VAR_TYPE_LONG:
            llvmType = llvm::Type::getInt64Ty(context);
            break;
        case VAR_TYPE_FLOAT:
            llvmType = llvm::Type::getFloatTy(context);
            break;
        case VAR_TYPE_DOUBLE:
            llvmType = llvm::Type::getDoubleTy(context);
            break;
        case VAR_TYPE_PTR:
            if (pointTo == nullptr) {
                std::cerr << "Pointer type is null" << std::endl;
                return nullptr;
            }
            llvmType = llvm::PointerType::get(pointTo->getLLVMType(context), 0);
            break;
        case VAR_TYPE_DEFINED:
            // TODO: Handle typedef names
            break;
        default:
            std::cerr << "Unknown type: " << type << std::endl;
            break;
    }

    return llvmType;
}