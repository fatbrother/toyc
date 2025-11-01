#include "ast/node.hpp"

#include <iostream>

#include "ast/structure.hpp"

using namespace toyc::ast;

CodegenResult NType::getLLVMType(ASTContext &context) const {
    llvm::Type *llvmType = nullptr;

    switch (type) {
        case VAR_TYPE_VOID:
            llvmType = llvm::Type::getVoidTy(context.llvmContext);
            break;
        case VAR_TYPE_CHAR:
            llvmType = llvm::Type::getInt8Ty(context.llvmContext);
            break;
        case VAR_TYPE_BOOL:
            llvmType = llvm::Type::getInt1Ty(context.llvmContext);
            break;
        case VAR_TYPE_SHORT:
            llvmType = llvm::Type::getInt16Ty(context.llvmContext);
            break;
        case VAR_TYPE_INT:
            llvmType = llvm::Type::getInt32Ty(context.llvmContext);
            break;
        case VAR_TYPE_LONG:
            llvmType = llvm::Type::getInt64Ty(context.llvmContext);
            break;
        case VAR_TYPE_FLOAT:
            llvmType = llvm::Type::getFloatTy(context.llvmContext);
            break;
        case VAR_TYPE_DOUBLE:
            llvmType = llvm::Type::getDoubleTy(context.llvmContext);
            break;
        case VAR_TYPE_PTR: {
                if (pointTo == nullptr) {
                    return CodegenResult("Pointer type with null pointTo");
                }

                CodegenResult pointToTypeResult = pointTo->getLLVMType(context);
                if (false == pointToTypeResult.isSuccess() || nullptr == pointToTypeResult.getLLVMType()) {
                    return pointToTypeResult << CodegenResult("Failed to get LLVM type for pointer pointTo");
                }

                llvmType = pointToTypeResult.getLLVMType();
                for (int i = 0; i < pointerLevel; ++i) {
                    CodegenResult ptrTypeResult = pointTo->getLLVMType(context);
                    if (false == ptrTypeResult.isSuccess() || nullptr == ptrTypeResult.getLLVMType()) {
                        return ptrTypeResult << CodegenResult("Failed to get LLVM type for pointer pointTo");
                    }
                    llvmType = llvm::PointerType::get(ptrTypeResult.getLLVMType(), 0);
                }
            }
            break;
        case VAR_TYPE_STRUCT: {
                if (name.empty()) {
                    return CodegenResult("Struct type with empty name");
                }
                auto [isFound, typePtr] = context.typeTable->lookup(name);
                if (false == isFound) {
                    return CodegenResult("Struct type not found in type table: " + name);
                }

                llvmType = static_cast<NStructType *>(typePtr.get())->llvmStructType;
                if (llvmType == nullptr) {
                    return CodegenResult("LLVM struct type is null for struct: " + name);
                }
            }
            break;
        case VAR_TYPE_DEFINED:
            // TODO: Handle typedef names
            break;
        default:
            return CodegenResult("Unsupported type for LLVM type generation: " + varTypeToString(type));
    }

    return CodegenResult(llvmType);
}