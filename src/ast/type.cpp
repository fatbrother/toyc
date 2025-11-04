#include "ast/type.hpp"

#include "ast/node.hpp"
#include "ast/expression.hpp"

#include <iostream>

using namespace toyc::ast;

TypeCodegenResult NType::getLLVMType(ASTContext &context) {
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
        case VAR_TYPE_DEFINED:
            // TODO: Handle typedef names
            break;
        default:
            return TypeCodegenResult("Unsupported type for LLVM type generation: " + varTypeToString(type));
    }

    return TypeCodegenResult(llvmType);
}

NTypePtr NType::getAddrType() {
    return std::make_shared<NPointerType>(std::shared_ptr<NType>(this), 1);
}

NArrayType::NArrayType(NTypePtr elementType, const std::vector<int> &dimensions)
    : NType(VAR_TYPE_ARRAY), elementType(elementType), arrayDimensions(dimensions) {}

NArrayType::NArrayType(NType *elementType, const std::vector<int> &dimensions)
    : NArrayType(std::shared_ptr<NType>(elementType), dimensions) {}

TypeCodegenResult NArrayType::getLLVMType(ASTContext &context) {
    TypeCodegenResult elemResult = elementType->getLLVMType(context);
    if (false == elemResult.isSuccess()) {
        return TypeCodegenResult("Failed to get LLVM type for array element type") << elemResult;
    }

    llvm::Type *arrayType = elemResult.getLLVMType();
    for (auto it = arrayDimensions.rbegin(); it != arrayDimensions.rend(); ++it) {
        arrayType = llvm::ArrayType::get(arrayType, *it);
    }
    return TypeCodegenResult(arrayType);
}

NTypePtr NArrayType::getElementType() const {
    if (arrayDimensions.size() == 1) {
        return elementType;
    } else {
        std::vector<int> innerDimensions(arrayDimensions.begin() + 1, arrayDimensions.end());
        return std::make_shared<NArrayType>(elementType, innerDimensions);
    }
}

const std::vector<int>& NArrayType::getArrayDimensions() const {
    return arrayDimensions;
}

int NArrayType::getTotalArraySize() const {
    int total = 1;
    for (int dim : arrayDimensions) {
        total *= dim;
    }
    return total;
}

std::string toyc::ast::varTypeToString(VarType type) {
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
        case VAR_TYPE_ARRAY: return "array";
        default: return "unknown";
    }
}

bool toyc::ast::isFloatingPointType(VarType type) {
    return type == VAR_TYPE_FLOAT || type == VAR_TYPE_DOUBLE;
}

bool toyc::ast::isIntegerType(VarType type) {
    return type == VAR_TYPE_CHAR || type == VAR_TYPE_BOOL ||
           type == VAR_TYPE_SHORT || type == VAR_TYPE_INT || type == VAR_TYPE_LONG;
}

// NPointerType implementation
NPointerType::NPointerType(NTypePtr pointeeType, int level)
    : NType(VAR_TYPE_PTR), pointeeType(pointeeType), level(level) {
        if (true == pointeeType->isPointer()) {
            auto innerPtrType = std::static_pointer_cast<NPointerType>(pointeeType);
            this->level += innerPtrType->level;
            this->pointeeType = innerPtrType->pointeeType;
        }
    }

NPointerType::NPointerType(VarType pointeeType, int level)
    : NPointerType(std::make_shared<NType>(pointeeType), level) {}

TypeCodegenResult NPointerType::getLLVMType(ASTContext &context) {
    if (pointeeType == nullptr) {
        return TypeCodegenResult("Pointer type with null pointee type");
    }

    TypeCodegenResult pointeeTypeResult = pointeeType->getLLVMType(context);
    if (!pointeeTypeResult.isSuccess()) {
        return TypeCodegenResult("Failed to get LLVM type for pointer pointee") << pointeeTypeResult;
    }

    llvm::Type *llvmType = pointeeTypeResult.getLLVMType();
    for (int i = 0; i < level; ++i) {
        llvmType = llvm::PointerType::get(llvmType, 0);
    }

    return TypeCodegenResult(llvmType);
}

NTypePtr NPointerType::getElementType() const {
    if (level > 1) {
        return std::make_shared<NPointerType>(pointeeType, level - 1);
    } else {
        return pointeeType;
    }
}

NTypePtr NPointerType::getAddrType() {
    return std::make_shared<NPointerType>(std::shared_ptr<NType>(this), level + 1);
}

NStructDeclaration::~NStructDeclaration() {
    SAFE_DELETE(declarator);
    SAFE_DELETE(next);
}

// NStructType implementation
NStructType::NStructType(const std::string &name, NStructDeclaration *members)
    : NType(VarType::VAR_TYPE_STRUCT), name(name), members(members) {}

NStructType::~NStructType() {
    SAFE_DELETE(members);
}

TypeCodegenResult NStructType::getLLVMType(ASTContext &context) {
    if (llvmStructType != nullptr) {
        return TypeCodegenResult(llvmStructType);
    }

    auto [isFound, typePtr] = context.typeTable->lookup(name);
    if (isFound) {
        auto existingStruct = std::static_pointer_cast<NStructType>(typePtr);
        if (existingStruct->llvmStructType != nullptr) {
            llvmStructType = existingStruct->llvmStructType;
            return TypeCodegenResult(llvmStructType);
        }
    }

    if (members == nullptr) {
        return TypeCodegenResult("Struct has no members: " + name);
    }

    std::vector<llvm::Type *> memberTypes;
    for (auto currentMember = members; currentMember != nullptr; currentMember = currentMember->next) {
        TypeCodegenResult typeResult = currentMember->type->getLLVMType(context);
        if (false == typeResult.isSuccess()) {
            return TypeCodegenResult("Failed to get LLVM type for member in struct: " + name) << typeResult;
        }
        llvm::Type *memberType = typeResult.getLLVMType();
        if (memberType == nullptr) {
            return TypeCodegenResult("Member type is null in struct: " + name);
        }
        memberTypes.push_back(memberType);
    }

    llvmStructType = llvm::StructType::create(context.llvmContext, memberTypes, name);
    if (llvmStructType == nullptr) {
        return TypeCodegenResult("Failed to create struct type: " + name);
    }

    return TypeCodegenResult(llvmStructType);
}

int NStructType::getMemberIndex(const std::string &memberName) const {
    int index = 0;
    for (NStructDeclaration *currentMember = members; currentMember != nullptr; currentMember = currentMember->next) {
        if (currentMember->declarator->getName() == memberName) {
            return index;
        }
        index++;
    }
    return -1;
}

NTypePtr NStructType::getMemberType(const std::string &memberName) const {
    for (NStructDeclaration *currentMember = members; currentMember != nullptr; currentMember = currentMember->next) {
        if (currentMember->declarator->getName() == memberName) {
            return currentMember->type;
        }
    }
    return nullptr;
}
