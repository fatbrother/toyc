#include "utility/type_cast.hpp"
#include "ast/node.hpp"

#include <llvm/IR/Type.h>
#include <llvm/IR/Constants.h>

using namespace toyc::ast;

namespace toyc::utility {

ExprCodegenResult castFromBool(llvm::Value *value, VarType toType, ASTContext &context) {
    llvm::Value *result = nullptr;
    NTypePtr resultType = context.typeFactory->getBasicType(toType);

    switch (toType) {
        case VAR_TYPE_CHAR:
            result = context.builder.CreateZExt(value, llvm::Type::getInt8Ty(context.llvmContext), "bool_to_char");
            break;
        case VAR_TYPE_SHORT:
            result = context.builder.CreateZExt(value, llvm::Type::getInt16Ty(context.llvmContext), "bool_to_short");
            break;
        case VAR_TYPE_INT:
            result = context.builder.CreateZExt(value, llvm::Type::getInt32Ty(context.llvmContext), "bool_to_int");
            break;
        case VAR_TYPE_LONG:
            result = context.builder.CreateZExt(value, llvm::Type::getInt64Ty(context.llvmContext), "bool_to_long");
            break;
        case VAR_TYPE_FLOAT:
            result = context.builder.CreateUIToFP(value, llvm::Type::getFloatTy(context.llvmContext), "bool_to_float");
            break;
        case VAR_TYPE_DOUBLE:
            result = context.builder.CreateUIToFP(value, llvm::Type::getDoubleTy(context.llvmContext), "bool_to_double");
            break;
        default:
            return ExprCodegenResult("Unsupported cast from bool to " + std::to_string(toType));
    }

    return ExprCodegenResult(result, resultType);
}

ExprCodegenResult castToBool(llvm::Value *value, VarType fromType, ASTContext &context) {
    llvm::Value *result = nullptr;
    NTypePtr resultType = context.typeFactory->getBasicType(VAR_TYPE_BOOL);

    if (isFloatingPointType(fromType)) {
        result = context.builder.CreateFCmpONE(value, llvm::ConstantFP::get(value->getType(), 0.0), "to_bool");
    } else {
        result = context.builder.CreateICmpNE(value, llvm::ConstantInt::get(value->getType(), 0), "to_bool");
    }

    return ExprCodegenResult(result, resultType);
}

ExprCodegenResult typeCast(llvm::Value *value, const NTypePtr fromType,
                      const NTypePtr toType, ASTContext &context) {
    if (nullptr == value || nullptr == fromType || nullptr == toType) {
        return ExprCodegenResult("Type cast failed due to null value or type");
    }

    VarType from = fromType->getVarType();
    VarType to = toType->getVarType();

    if (from == to) {
        return ExprCodegenResult(value, toType);
    }

    if (from == VAR_TYPE_BOOL) {
        return castFromBool(value, to, context);
    }

    if (to == VAR_TYPE_BOOL) {
        return castToBool(value, from, context);
    }

    llvm::Value *result = nullptr;
    switch (to) {
        case VAR_TYPE_CHAR:
            if (isFloatingPointType(from)) {
                result = context.builder.CreateFPToSI(value, llvm::Type::getInt8Ty(context.llvmContext), "to_char");
            } else {
                result = context.builder.CreateIntCast(value, llvm::Type::getInt8Ty(context.llvmContext), true, "to_char");
            }
            break;
        case VAR_TYPE_SHORT:
            if (isFloatingPointType(from)) {
                result = context.builder.CreateFPToSI(value, llvm::Type::getInt16Ty(context.llvmContext), "to_short");
            } else {
                result = context.builder.CreateIntCast(value, llvm::Type::getInt16Ty(context.llvmContext), true, "to_short");
            }
            break;
        case VAR_TYPE_INT:
            if (isFloatingPointType(from)) {
                result = context.builder.CreateFPToSI(value, llvm::Type::getInt32Ty(context.llvmContext), "to_int");
            } else {
                result = context.builder.CreateIntCast(value, llvm::Type::getInt32Ty(context.llvmContext), true, "to_int");
            }
            break;
        case VAR_TYPE_LONG:
            if (isFloatingPointType(from)) {
                result = context.builder.CreateFPToSI(value, llvm::Type::getInt64Ty(context.llvmContext), "to_long");
            } else {
                result = context.builder.CreateIntCast(value, llvm::Type::getInt64Ty(context.llvmContext), true, "to_long");
            }
            break;
        case VAR_TYPE_FLOAT:
            if (isIntegerType(from)) {
                result = context.builder.CreateSIToFP(value, llvm::Type::getFloatTy(context.llvmContext), "to_float");
            } else {
                result = context.builder.CreateFPCast(value, llvm::Type::getFloatTy(context.llvmContext), "to_float");
            }
            break;
        case VAR_TYPE_DOUBLE:
            if (isIntegerType(from)) {
                result = context.builder.CreateSIToFP(value, llvm::Type::getDoubleTy(context.llvmContext), "to_double");
            } else {
                result = context.builder.CreateFPCast(value, llvm::Type::getDoubleTy(context.llvmContext), "to_double");
            }
            break;
        case VAR_TYPE_PTR:
            if (isIntegerType(from)) {
                auto ptrType = std::static_pointer_cast<NPointerType>(toType);
                result = context.builder.CreateIntToPtr(
                    value,
                    ptrType->getLLVMType(context).getLLVMType(),
                    "to_ptr");
            } else {
                result = context.builder.CreateBitCast(value, toType->getLLVMType(context).getLLVMType(), "to_ptr");
            }
            break;
        default:
            return ExprCodegenResult("Unsupported type cast from " + varTypeToString(from) + " to " + varTypeToString(to));
    }

    return ExprCodegenResult(result, toType);
}

ExprCodegenResult typeCast(llvm::Value *value, const NTypePtr fromType,
                      VarType toType, ASTContext &context) {
    NTypePtr toNType = context.typeFactory->getBasicType(toType);
    return typeCast(value, fromType, toNType, context);
}

} // namespace toyc::utility
