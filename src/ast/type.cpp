#include "ast/type.hpp"
#include "ast/node.hpp"
#include "ast/expression.hpp"

#include <llvm/IR/Type.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Module.h>

#include <iostream>

using namespace toyc::ast;

// ==================== Helper Functions ====================

bool toyc::ast::isFloatingPointType(llvm::Type* type) {
    return type && (type->isFloatTy() || type->isDoubleTy());
}

bool toyc::ast::isIntegerType(llvm::Type* type) {
    return type && type->isIntegerTy();
}

NStructDeclaration::~NStructDeclaration() {
    SAFE_DELETE(declarator);
    SAFE_DELETE(next);
}

// ==================== StructTypeDescriptor ====================

StructTypeDescriptor::StructTypeDescriptor(std::string name, NStructDeclaration* members)
    : name(std::move(name)), members(members) {}

StructTypeDescriptor::~StructTypeDescriptor() {
    SAFE_DELETE(members);
}

// ==================== StructMetadata ====================

StructMetadata::~StructMetadata() {
    SAFE_DELETE(members);
}

// ==================== TypeManager Implementation ====================

TypeManager::TypeManager(llvm::LLVMContext& ctx, llvm::Module& mod)
    : context(ctx), module(mod) {}

llvm::Type* TypeManager::getVoidType() {
    return llvm::Type::getVoidTy(context);
}

llvm::Type* TypeManager::getBoolType() {
    return llvm::Type::getInt1Ty(context);
}

llvm::Type* TypeManager::getCharType() {
    return llvm::Type::getInt8Ty(context);
}

llvm::Type* TypeManager::getShortType() {
    return llvm::Type::getInt16Ty(context);
}

llvm::Type* TypeManager::getIntType() {
    return llvm::Type::getInt32Ty(context);
}

llvm::Type* TypeManager::getLongType() {
    return llvm::Type::getInt64Ty(context);
}

llvm::Type* TypeManager::getFloatType() {
    return llvm::Type::getFloatTy(context);
}

llvm::Type* TypeManager::getDoubleType() {
    return llvm::Type::getDoubleTy(context);
}

llvm::Type* TypeManager::getBasicType(VarType type) {
    switch (type) {
        case VAR_TYPE_VOID: return getVoidType();
        case VAR_TYPE_BOOL: return getBoolType();
        case VAR_TYPE_CHAR: return getCharType();
        case VAR_TYPE_SHORT: return getShortType();
        case VAR_TYPE_INT: return getIntType();
        case VAR_TYPE_LONG: return getLongType();
        case VAR_TYPE_FLOAT: return getFloatType();
        case VAR_TYPE_DOUBLE: return getDoubleType();
        default: return nullptr;
    }
}

llvm::PointerType* TypeManager::getPointerType(llvm::Type* pointeeType) {
    return getPointerType(pointeeType, 1);
}

llvm::PointerType* TypeManager::getPointerType(llvm::Type* pointeeType, int level) {
    if (!pointeeType || level <= 0) {
        return nullptr;
    }

    llvm::Type* result = pointeeType;
    for (int i = 0; i < level; ++i) {
        result = llvm::PointerType::get(result, 0);
    }

    pointerMetadataMap[result] = pointeeType;
    return llvm::cast<llvm::PointerType>(result);
}

llvm::ArrayType* TypeManager::getArrayType(llvm::Type* elementType, const std::vector<int>& dimensions) {
    if (!elementType || dimensions.empty()) {
        return nullptr;
    }

    llvm::Type* arrayType = elementType;
    for (auto dimIt = dimensions.rbegin(); dimIt != dimensions.rend(); ++dimIt) {
        arrayType = llvm::ArrayType::get(arrayType, *dimIt);
    }

    if (arrayType) {
        arrayMetadataMap[arrayType] = {dimensions, elementType};
    }

    return llvm::cast<llvm::ArrayType>(arrayType);
}

const ArrayMetadata* TypeManager::getArrayMetadata(llvm::Type* arrayType) const {
    auto it = arrayMetadataMap.find(arrayType);
    if (it != arrayMetadataMap.end()) {
        return &it->second;
    }
    return nullptr;
}

llvm::StructType* TypeManager::createStructType(const std::string& name,
                                                NStructDeclaration* members,
                                                ASTContext& context) {
    llvm::StructType* llvmStruct = llvm::StructType::create(this->context, name);

    if (nullptr == members) {
        return llvmStruct;
    }

    std::vector<llvm::Type*> memberLLVMTypes;
    NStructDeclaration* current = members;

    auto metadata = std::make_shared<StructMetadata>();
    metadata->name = name;
    metadata->members = members;

    for (int index = 0; current != nullptr; index++, current = current->next) {
        llvm::Type* memberType = realize(current->type, context);
        if (nullptr == memberType) {
            break;
        }

        memberLLVMTypes.push_back(memberType);
        metadata->memberTypes.push_back(memberType);
        std::string memberName = current->declarator->getName();
        metadata->memberIndexMap[memberName] = index;
    }

    llvmStruct->setBody(memberLLVMTypes);

    structMetadataByType[llvmStruct] = metadata;

    return llvmStruct;
}

llvm::StructType* TypeManager::getStructType(const std::string& name) {
    return llvm::StructType::getTypeByName(module.getContext(), name);
}

const std::shared_ptr<StructMetadata> TypeManager::getStructMetadata(llvm::StructType* structType) const {
    auto it = structMetadataByType.find(structType);
    if (it != structMetadataByType.end()) {
        return it->second;
    }
    return nullptr;
}

llvm::Type* TypeManager::getPointeeType(llvm::Type* pointerType) {
    if (nullptr == pointerType || false == pointerType->isPointerTy()) {
        return nullptr;
    }

    return pointerMetadataMap[pointerType];
}

llvm::Type* TypeManager::getArrayElementType(llvm::Type* arrayType) {
    if (nullptr == arrayType || false == arrayType->isArrayTy()) {
        return nullptr;
    }
    return llvm::cast<llvm::ArrayType>(arrayType)->getElementType();
}

std::string TypeManager::getTypeName(llvm::Type* type) const {
    if (!type) {
        return "null";
    }

    if (type->isIntegerTy(1)) return "bool";
    if (type->isIntegerTy(8)) return "char";
    if (type->isIntegerTy(16)) return "short";
    if (type->isIntegerTy(32)) return "int";
    if (type->isIntegerTy(64)) return "long";
    if (type->isFloatTy()) return "float";
    if (type->isDoubleTy()) return "double";
    if (type->isVoidTy()) return "void";

    if (type->isPointerTy()) {
        return "ptr";
    }

    if (type->isArrayTy()) {
        auto* arrayType = llvm::cast<llvm::ArrayType>(type);
        return "array[" + std::to_string(arrayType->getNumElements()) + "]";
    }

    if (auto* structType = llvm::dyn_cast<llvm::StructType>(type)) {
        if (structType->hasName()) {
            return structType->getName().str();
        }
        return "struct";
    }

    return "unknown";
}

llvm::Type* TypeManager::getCommonType(llvm::Type* type1, llvm::Type* type2) {
    if (nullptr == type1 || nullptr == type2) {
        return nullptr;
    }

    if (type1 == type2) return type1;

    if (true == isFloatingPointType(type1) && true == isFloatingPointType(type2)) {
        if (type1->isDoubleTy() || type2->isDoubleTy()) {
            return getDoubleType();
        }
        return getFloatType();
    }

    auto getIntegerRank = [](llvm::Type* type) -> int {
        if (type->isIntegerTy(64)) return 5; // long
        if (type->isIntegerTy(32)) return 4; // int
        if (type->isIntegerTy(16)) return 3; // short
        if (type->isIntegerTy(8)) return 2;  // char
        if (type->isIntegerTy(1)) return 1;  // bool
        return 0;
    };

    return (getIntegerRank(type1) >= getIntegerRank(type2)) ? type1 : type2;
}

llvm::Type* TypeManager::realize(const TypeDescriptor* descriptor, ASTContext& context) {
    if (!descriptor) {
        return nullptr;
    }

    switch (descriptor->getKind()) {
        case TypeDescriptor::Primitive: {
            auto* primDesc = static_cast<const PrimitiveTypeDescriptor*>(descriptor);
            return getBasicType(primDesc->varType);
        }

        case TypeDescriptor::Pointer: {
            auto* ptrDesc = static_cast<const PointerTypeDescriptor*>(descriptor);
            llvm::Type* pointeeType = realize(ptrDesc->pointeeDesc.get(), context);
            if (nullptr == pointeeType) {
                return nullptr;
            }
            return getPointerType(pointeeType, ptrDesc->level);
        }

        case TypeDescriptor::Array: {
            auto* arrayDesc = static_cast<const ArrayTypeDescriptor*>(descriptor);
            llvm::Type* elementType = realize(arrayDesc->elementDesc.get(), context);
            if (nullptr == elementType) {
                return nullptr;
            }
            return getArrayType(elementType, arrayDesc->dimensions);
        }

        case TypeDescriptor::Struct: {
            auto* structDesc = static_cast<const StructTypeDescriptor*>(descriptor);

            llvm::StructType* existingStruct = getStructType(structDesc->name);
            if (existingStruct && false == existingStruct->isOpaque()) {
                return existingStruct;
            }

            return createStructType(structDesc->name, structDesc->members, context);
        }

        default:
            return nullptr;
    }
}

