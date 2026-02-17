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

// ==================== NStructDeclaration ====================

NStructDeclaration::~NStructDeclaration() {
    SAFE_DELETE(declarator);
    SAFE_DELETE(next);
}

// ==================== StructTypeCodegen ====================

StructTypeCodegen::~StructTypeCodegen() {
    SAFE_DELETE(members);
}

// ==================== TypeCodegen getLLVMType implementations ====================

llvm::Type* PrimitiveTypeCodegen::getLLVMType(TypeManager& tm) {
    return tm.getBasicType(varType);
}

llvm::Type* PointerTypeCodegen::getLLVMType(TypeManager& tm) {
    llvm::Type* pointeeLLVMType = tm.realize(pointeeIdx);
    if (!pointeeLLVMType) return nullptr;

    llvm::Type* result = pointeeLLVMType;
    for (int i = 0; i < level; ++i)
        result = llvm::PointerType::get(result, 0);

    tm.registerPointerMetadata(result, pointeeLLVMType);
    return llvm::cast<llvm::PointerType>(result);
}

llvm::Type* ArrayTypeCodegen::getLLVMType(TypeManager& tm) {
    llvm::Type* elemLLVMType = tm.realize(elementIdx);
    if (!elemLLVMType) return nullptr;

    llvm::Type* arrayType = elemLLVMType;
    for (auto dimIt = dimensions.rbegin(); dimIt != dimensions.rend(); ++dimIt)
        arrayType = llvm::ArrayType::get(arrayType, *dimIt);

    return llvm::cast<llvm::ArrayType>(arrayType);
}

llvm::Type* StructTypeCodegen::getLLVMType(TypeManager& tm) {
    llvm::StructType* existing = tm.getStructType(name);
    if (existing && !existing->isOpaque()) return existing;

    llvm::StructType* llvmStruct = llvm::StructType::create(tm.getLLVMContext(), name);
    if (!members) return llvmStruct;

    std::vector<llvm::Type*> memberLLVMTypes;
    auto metadata = std::make_shared<StructMetadata>();
    metadata->name = name;

    int index = 0;
    for (NStructDeclaration* cur = members; cur != nullptr; cur = cur->next, ++index) {
        llvm::Type* memberType = tm.realize(cur->typeIdx);
        if (!memberType) break;
        memberLLVMTypes.push_back(memberType);
        metadata->memberTypes.push_back(memberType);
        metadata->memberIndexMap[cur->declarator->getName()] = index;
    }

    llvmStruct->setBody(memberLLVMTypes);
    tm.registerStructMetadata(llvmStruct, metadata);
    return llvmStruct;
}

// ==================== StructMetadata ====================

// ==================== TypeManager ====================

TypeManager::TypeManager(llvm::LLVMContext& ctx, llvm::Module& mod)
    : context(ctx), module(mod) {}

TypeIdx TypeManager::registerType(const TypeKey& key, std::unique_ptr<TypeCodegen> node) {
    auto it = cache_.find(key);
    if (it != cache_.end()) return it->second;
    TypeIdx idx = static_cast<TypeIdx>(types_.size());
    cache_[key] = idx;
    types_.push_back(std::move(node));
    return idx;
}

TypeIdx TypeManager::getPrimitiveIdx(VarType vt) {
    TypeKey key;
    key.kind = TypeKey::Primitive;
    key.varType = vt;
    return registerType(key, std::make_unique<PrimitiveTypeCodegen>(vt));
}

TypeIdx TypeManager::getPointerIdx(TypeIdx pointee, int level) {
    TypeKey key;
    key.kind = TypeKey::Pointer;
    key.pointeeIdx = pointee;
    key.level = level;
    return registerType(key, std::make_unique<PointerTypeCodegen>(pointee, level));
}

TypeIdx TypeManager::getArrayIdx(TypeIdx elem, std::vector<int> dims) {
    TypeKey key;
    key.kind = TypeKey::Array;
    key.elementIdx = elem;
    key.dims = dims;
    return registerType(key, std::make_unique<ArrayTypeCodegen>(elem, std::move(dims)));
}

TypeIdx TypeManager::getStructIdx(const std::string& name, NStructDeclaration* members) {
    TypeKey key;
    key.kind = TypeKey::Struct;
    key.structName = name;
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        if (members != nullptr) {
            StructTypeCodegen* existing = static_cast<StructTypeCodegen*>(types_[it->second].get());
            if (existing->getMembers() == nullptr) {
                existing->setMembers(members);
            }
        }
        return it->second;
    }
    return registerType(key, std::make_unique<StructTypeCodegen>(name, members));
}

const TypeCodegen* TypeManager::get(TypeIdx idx) const {
    if (idx == InvalidTypeIdx || idx >= static_cast<TypeIdx>(types_.size())) return nullptr;
    return types_[idx].get();
}

llvm::Type* TypeManager::realize(TypeIdx idx) {
    if (idx == InvalidTypeIdx || idx >= static_cast<TypeIdx>(types_.size())) return nullptr;
    return types_[idx]->getLLVMType(*this);
}

llvm::Type* TypeManager::getBasicType(VarType type) {
    switch (type) {
        case VAR_TYPE_VOID:   return llvm::Type::getVoidTy(context);
        case VAR_TYPE_BOOL:   return llvm::Type::getInt1Ty(context);
        case VAR_TYPE_CHAR:   return llvm::Type::getInt8Ty(context);
        case VAR_TYPE_SHORT:  return llvm::Type::getInt16Ty(context);
        case VAR_TYPE_INT:    return llvm::Type::getInt32Ty(context);
        case VAR_TYPE_LONG:   return llvm::Type::getInt64Ty(context);
        case VAR_TYPE_FLOAT:  return llvm::Type::getFloatTy(context);
        case VAR_TYPE_DOUBLE: return llvm::Type::getDoubleTy(context);
        default: return nullptr;
    }
}

llvm::StructType* TypeManager::getStructType(const std::string& name) {
    return llvm::StructType::getTypeByName(module.getContext(), name);
}

void TypeManager::registerPointerMetadata(llvm::Type* ptrType, llvm::Type* pointeeType) {
    pointerMetadataMap[ptrType] = pointeeType;
}

void TypeManager::registerStructMetadata(llvm::StructType* structType, std::shared_ptr<StructMetadata> meta) {
    structMetadataByType[structType] = std::move(meta);
}

// ==================== LLVM-level helpers ====================

llvm::Type* TypeManager::getPointeeType(llvm::Type* pointerType) {
    if (!pointerType || !pointerType->isPointerTy()) return nullptr;
    return pointerMetadataMap[pointerType];
}

const std::shared_ptr<StructMetadata> TypeManager::getStructMetadata(llvm::StructType* structType) const {
    auto it = structMetadataByType.find(structType);
    return (it != structMetadataByType.end()) ? it->second : nullptr;
}

std::string TypeManager::getTypeName(llvm::Type* type) const {
    if (!type) return "null";
    if (type->isIntegerTy(1))  return "bool";
    if (type->isIntegerTy(8))  return "char";
    if (type->isIntegerTy(16)) return "short";
    if (type->isIntegerTy(32)) return "int";
    if (type->isIntegerTy(64)) return "long";
    if (type->isFloatTy())     return "float";
    if (type->isDoubleTy())    return "double";
    if (type->isVoidTy())      return "void";
    if (type->isPointerTy())   return "ptr";
    if (type->isArrayTy()) {
        auto* arrayType = llvm::cast<llvm::ArrayType>(type);
        return "array[" + std::to_string(arrayType->getNumElements()) + "]";
    }
    if (auto* structType = llvm::dyn_cast<llvm::StructType>(type)) {
        if (structType->hasName()) return structType->getName().str();
        return "struct";
    }
    return "unknown";
}

llvm::Type* TypeManager::getCommonType(llvm::Type* type1, llvm::Type* type2) {
    if (!type1 || !type2) return nullptr;
    if (type1 == type2) return type1;

    if (isFloatingPointType(type1) && isFloatingPointType(type2)) {
        if (type1->isDoubleTy() || type2->isDoubleTy()) return getBasicType(VAR_TYPE_DOUBLE);
        return getBasicType(VAR_TYPE_FLOAT);
    }

    auto getIntegerRank = [](llvm::Type* t) -> int {
        if (t->isIntegerTy(64)) return 5;
        if (t->isIntegerTy(32)) return 4;
        if (t->isIntegerTy(16)) return 3;
        if (t->isIntegerTy(8))  return 2;
        if (t->isIntegerTy(1))  return 1;
        return 0;
    };
    return (getIntegerRank(type1) >= getIntegerRank(type2)) ? type1 : type2;
}
