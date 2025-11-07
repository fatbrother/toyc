#include "ast/type.hpp"

#include "ast/node.hpp"
#include "ast/expression.hpp"

#include <iostream>

using namespace toyc::ast;

TypeCodegenResult NType::getLLVMType(ASTContext &context) {
    if (cachedLLVMType != nullptr) {
        return TypeCodegenResult(cachedLLVMType);
    }

    llvm::Type* llvmType = generateLLVMType(context);
    if (llvmType == nullptr) {
        return TypeCodegenResult("Failed to generate LLVM type for " + getName());
    }

    cachedLLVMType = llvmType;
    return TypeCodegenResult(llvmType);
}

llvm::Type* NType::generateLLVMType(ASTContext &context) {
    switch (type) {
        case VAR_TYPE_VOID:
            return llvm::Type::getVoidTy(context.llvmContext);
        case VAR_TYPE_CHAR:
            return llvm::Type::getInt8Ty(context.llvmContext);
        case VAR_TYPE_BOOL:
            return llvm::Type::getInt1Ty(context.llvmContext);
        case VAR_TYPE_SHORT:
            return llvm::Type::getInt16Ty(context.llvmContext);
        case VAR_TYPE_INT:
            return llvm::Type::getInt32Ty(context.llvmContext);
        case VAR_TYPE_LONG:
            return llvm::Type::getInt64Ty(context.llvmContext);
        case VAR_TYPE_FLOAT:
            return llvm::Type::getFloatTy(context.llvmContext);
        case VAR_TYPE_DOUBLE:
            return llvm::Type::getDoubleTy(context.llvmContext);
        case VAR_TYPE_DEFINED:
            // TODO: Handle typedef names
            return nullptr;
        default:
            return nullptr;
    }
}

NTypePtr NType::getAddrType(ASTContext& context) {
    return context.typeFactory->getPointerType(shared_from_this());
}

NArrayType::NArrayType(NTypePtr elementType, const std::vector<int> &dimensions)
    : NType(VAR_TYPE_ARRAY), elementType(elementType), arrayDimensions(dimensions) {}

llvm::Type* NArrayType::generateLLVMType(ASTContext &context) {
    TypeCodegenResult elemResult = elementType->getLLVMType(context);
    if (!elemResult.isSuccess()) {
        return nullptr;
    }

    llvm::Type *arrayType = elemResult.getLLVMType();
    for (auto it = arrayDimensions.rbegin(); it != arrayDimensions.rend(); ++it) {
        arrayType = llvm::ArrayType::get(arrayType, *it);
    }
    return arrayType;
}

NTypePtr NArrayType::getElementType(ASTContext& context) const {
    if (arrayDimensions.size() == 1) {
        return elementType;
    } else {
        std::vector<int> innerDimensions(arrayDimensions.begin() + 1, arrayDimensions.end());
        return context.typeFactory->getArrayType(elementType, innerDimensions);
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

llvm::Type* NPointerType::generateLLVMType(ASTContext &context) {
    if (pointeeType == nullptr) {
        return nullptr;
    }

    TypeCodegenResult pointeeTypeResult = pointeeType->getLLVMType(context);
    if (!pointeeTypeResult.isSuccess()) {
        return nullptr;
    }

    llvm::Type *llvmType = pointeeTypeResult.getLLVMType();
    for (int i = 0; i < level; ++i) {
        llvmType = llvm::PointerType::get(llvmType, 0);
    }

    return llvmType;
}

NTypePtr NPointerType::getElementType(ASTContext& context) const {
    if (level > 1) {
        return context.typeFactory->getPointerType(pointeeType, level - 1);
    } else {
        return pointeeType;
    }
}

NTypePtr NPointerType::getAddrType(ASTContext& context) {
    return context.typeFactory->getPointerType(pointeeType, level + 1);
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

llvm::Type* NStructType::generateLLVMType(ASTContext &context) {
    auto [isFound, typePtr] = context.typeTable->lookup(name);
    if (isFound) {
        auto existingStruct = std::static_pointer_cast<NStructType>(typePtr);
        TypeCodegenResult result = existingStruct->getLLVMType(context);
        if (result.isSuccess()) {
            return result.getLLVMType();
        }
    }

    if (members == nullptr) {
        return llvm::StructType::create(context.llvmContext, name);
    }

    std::vector<llvm::Type *> memberLLVMTypes;
    for (auto currentMember = members; currentMember != nullptr; currentMember = currentMember->next) {
        NTypePtr memberType = context.typeFactory->realize(currentMember->type);
        if (memberType == nullptr) {
            return nullptr;
        }
        TypeCodegenResult typeResult = memberType->getLLVMType(context);
        if (!typeResult.isSuccess()) {
            return nullptr;
        }
        llvm::Type *memberLLVMType = typeResult.getLLVMType();
        if (memberLLVMType == nullptr) {
            return nullptr;
        }
        memberLLVMTypes.push_back(memberLLVMType);
    }

    llvm::StructType* structType = llvm::StructType::create(context.llvmContext, memberLLVMTypes, name);
    return structType;
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

NTypePtr NStructType::getMemberType(const std::string &memberName, ASTContext& context) const {
    for (NStructDeclaration *currentMember = members; currentMember != nullptr; currentMember = currentMember->next) {
        if (currentMember->declarator->getName() == memberName) {
            return context.typeFactory->realize(currentMember->type);
        }
    }
    return nullptr;
}

// ==================== TypeDescriptor Implementation ====================

StructTypeDescriptor::StructTypeDescriptor(std::string name, NStructDeclaration* members)
    : name(std::move(name)), members(members) {}

StructTypeDescriptor::~StructTypeDescriptor() {
    SAFE_DELETE(members);
}

// ==================== TypeFactory Implementation ====================

TypeFactory::TypeFactory() {
    voidType = std::shared_ptr<NType>(new NType(VAR_TYPE_VOID));
    boolType = std::shared_ptr<NType>(new NType(VAR_TYPE_BOOL));
    charType = std::shared_ptr<NType>(new NType(VAR_TYPE_CHAR));
    shortType = std::shared_ptr<NType>(new NType(VAR_TYPE_SHORT));
    intType = std::shared_ptr<NType>(new NType(VAR_TYPE_INT));
    longType = std::shared_ptr<NType>(new NType(VAR_TYPE_LONG));
    floatType = std::shared_ptr<NType>(new NType(VAR_TYPE_FLOAT));
    doubleType = std::shared_ptr<NType>(new NType(VAR_TYPE_DOUBLE));
}

NTypePtr TypeFactory::getBasicType(VarType type) {
    switch (type) {
        case VAR_TYPE_VOID: return voidType;
        case VAR_TYPE_BOOL: return boolType;
        case VAR_TYPE_CHAR: return charType;
        case VAR_TYPE_SHORT: return shortType;
        case VAR_TYPE_INT: return intType;
        case VAR_TYPE_LONG: return longType;
        case VAR_TYPE_FLOAT: return floatType;
        case VAR_TYPE_DOUBLE: return doubleType;
        default: return nullptr;
    }
}

NTypePtr TypeFactory::getPointerType(NTypePtr pointeeType) {
    return getPointerType(pointeeType, 1);
}

NTypePtr TypeFactory::getPointerType(NTypePtr pointeeType, int level) {
    if (!pointeeType || level <= 0) {
        return nullptr;
    }

    size_t hash = computeTypeHash(pointeeType) * 31 + level;

    auto it = pointerTypeCache.find(hash);
    if (it != pointerTypeCache.end()) {
        return it->second;
    }

    NTypePtr ptrType = std::shared_ptr<NPointerType>(new NPointerType(pointeeType, level));
    pointerTypeCache[hash] = ptrType;
    return ptrType;
}

NTypePtr TypeFactory::getArrayType(NTypePtr elementType, const std::vector<int>& dimensions) {
    if (!elementType || dimensions.empty()) {
        return nullptr;
    }

    ArrayKey key{computeTypeHash(elementType), dimensions};
    auto it = arrayTypeCache.find(key);
    if (it != arrayTypeCache.end()) {
        return it->second;
    }

    NTypePtr arrayType = std::shared_ptr<NArrayType>(new NArrayType(elementType, dimensions));
    arrayTypeCache[key] = arrayType;
    return arrayType;
}

NTypePtr TypeFactory::createStructType(const std::string& name, NStructDeclaration* members) {
    NTypePtr structType = std::shared_ptr<NStructType>(new NStructType(name, members));
    if (false == name.empty()) {
        structTypeRegistry[name] = structType;
    }
    return structType;
}

void TypeFactory::registerStructType(const std::string& name, NTypePtr structType) {
    if (structType && structType->isStruct()) {
        structTypeRegistry[name] = structType;
    }
}

NTypePtr TypeFactory::getStructType(const std::string& name, NStructDeclaration* members) {
    auto it = structTypeRegistry.find(name);
    if (it != structTypeRegistry.end()) {
        return it->second;
    }

    return createStructType(name, members);
}

bool TypeFactory::hasStructType(const std::string& name) const {
    return structTypeRegistry.find(name) != structTypeRegistry.end();
}

NTypePtr TypeFactory::realize(const TypeDescriptor* descriptor) {
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
            NTypePtr pointeeType = realize(ptrDesc->pointeeDesc.get());
            if (!pointeeType) {
                return nullptr;
            }
            return getPointerType(pointeeType, ptrDesc->level);
        }

        case TypeDescriptor::Array: {
            auto* arrayDesc = static_cast<const ArrayTypeDescriptor*>(descriptor);
            NTypePtr elementType = realize(arrayDesc->elementDesc.get());
            if (!elementType) {
                return nullptr;
            }
            return getArrayType(elementType, arrayDesc->dimensions);
        }

        case TypeDescriptor::Struct: {
            auto* structDesc = static_cast<const StructTypeDescriptor*>(descriptor);
            return getStructType(structDesc->name, structDesc->members);
        }

        default:
            return nullptr;
    }
}

size_t TypeFactory::computeTypeHash(NTypePtr type) const {
    if (!type) {
        return 0;
    }

    return reinterpret_cast<size_t>(type.get());
}

