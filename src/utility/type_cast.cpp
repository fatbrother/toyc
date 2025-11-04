#include "utility/type_cast.hpp"
#include "ast/type.hpp"
#include "ast/node.hpp"

#include <llvm/IR/Type.h>
#include <llvm/IR/Constants.h>

using namespace toyc::ast;

namespace toyc::utility {

CodegenResult castFromBool(llvm::Value *value, VarType toType,
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

CodegenResult castToBool(llvm::Value *value, VarType fromType,
                       llvm::LLVMContext &context, llvm::IRBuilder<> &builder) {
    if (isFloatingPointType(fromType)) {
        return CodegenResult(builder.CreateFCmpONE(value, llvm::ConstantFP::get(value->getType(), 0.0), "to_bool"));
    }

    return CodegenResult(builder.CreateICmpNE(value, llvm::ConstantInt::get(value->getType(), 0), "to_bool"));
}

CodegenResult typeCast(llvm::Value *value, const NTypePtr fromType,
                      const NTypePtr toType, ASTContext &context) {
    if (nullptr == value || nullptr == fromType || nullptr == toType) {
        return CodegenResult("Type cast failed due to null value or type");
    }

    VarType from = fromType->getVarType();
    VarType to = toType->getVarType();

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
                return CodegenResult(context.builder.CreateFPToSI(value, llvm::Type::getInt8Ty(context.llvmContext), "to_char"));
            }
            return CodegenResult(context.builder.CreateIntCast(value, llvm::Type::getInt8Ty(context.llvmContext), true, "to_char"));
        case VAR_TYPE_SHORT:
            if (isFloatingPointType(from)) {
                return CodegenResult(context.builder.CreateFPToSI(value, llvm::Type::getInt16Ty(context.llvmContext), "to_short"));
            }
            return CodegenResult(context.builder.CreateIntCast(value, llvm::Type::getInt16Ty(context.llvmContext), true, "to_short"));
        case VAR_TYPE_INT:
            if (isFloatingPointType(from)) {
                return CodegenResult(context.builder.CreateFPToSI(value, llvm::Type::getInt32Ty(context.llvmContext), "to_int"));
            }
            return CodegenResult(context.builder.CreateIntCast(value, llvm::Type::getInt32Ty(context.llvmContext), true, "to_int"));
        case VAR_TYPE_LONG:
            if (isFloatingPointType(from)) {
                return CodegenResult(context.builder.CreateFPToSI(value, llvm::Type::getInt64Ty(context.llvmContext), "to_long"));
            }
            return CodegenResult(context.builder.CreateIntCast(value, llvm::Type::getInt64Ty(context.llvmContext), true, "to_long"));
        case VAR_TYPE_FLOAT:
            if (isIntegerType(from)) {
                return CodegenResult(context.builder.CreateSIToFP(value, llvm::Type::getFloatTy(context.llvmContext), "to_float"));
            }
            return CodegenResult(context.builder.CreateFPCast(value, llvm::Type::getFloatTy(context.llvmContext), "to_float"));
        case VAR_TYPE_DOUBLE:
            if (isIntegerType(from)) {
                return CodegenResult(context.builder.CreateSIToFP(value, llvm::Type::getDoubleTy(context.llvmContext), "to_double"));
            }
            return CodegenResult(context.builder.CreateFPCast(value, llvm::Type::getDoubleTy(context.llvmContext), "to_double"));
        case VAR_TYPE_PTR:
            if (isIntegerType(from)) {
                auto ptrType = std::static_pointer_cast<NPointerType>(toType);
                return CodegenResult(context.builder.CreateIntToPtr(
                    value,
                    ptrType->getLLVMType(context).getLLVMType(),
                    "to_ptr"));
            }
            return CodegenResult(context.builder.CreateBitCast(value, toType->getLLVMType(context).getLLVMType(), "to_ptr"));
        default:
            break;
    }

    return CodegenResult("Unsupported type cast from " + varTypeToString(from) + " to " + varTypeToString(to));
}

CodegenResult typeCast(llvm::Value *value, const NTypePtr fromType,
                      VarType toType, ASTContext &context) {
    NTypePtr toNType = std::make_shared<NType>(toType);
    return typeCast(value, fromType, toNType, context);
}

} // namespace toyc::utility
