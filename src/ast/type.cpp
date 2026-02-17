#include "ast/type.hpp"

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

#include <iostream>
#include <string>

#include "ast/expression.hpp"
#include "ast/node.hpp"

using namespace toyc::ast;

// ==================== NStructDeclaration ====================

NStructDeclaration::~NStructDeclaration() {
    SAFE_DELETE(declarator);
    SAFE_DELETE(next);
}

// ==================== StructTypeCodegen ====================

StructTypeCodegen::StructTypeCodegen(std::string name, NStructDeclaration* members) : name(std::move(name)) {
    setMembers(members);
    delete members;  // StructTypeCodegen takes ownership of members
}

void StructTypeCodegen::setMembers(NStructDeclaration* m) {
    memberInfos.clear();
    int index = 0;
    for (NStructDeclaration* cur = m; cur != nullptr; cur = cur->next, ++index) {
        memberInfos.push_back({cur->declarator->getName(), cur->typeIdx});
    }
}

int StructTypeCodegen::getMemberIndex(const std::string& memberName) const {
    for (size_t i = 0; i < memberInfos.size(); ++i) {
        if (memberInfos[i].name == memberName)
            return static_cast<int>(i);
    }
    return -1;
}

TypeIdx StructTypeCodegen::getMemberTypeIdx(int index) const {
    if (index >= 0 && static_cast<size_t>(index) < memberInfos.size())
        return memberInfos[index].typeIdx;
    return InvalidTypeIdx;
}

// ==================== TypeCodegen getLLVMType implementations ====================

llvm::Type* PrimitiveTypeCodegen::getLLVMType(TypeManager& tm, llvm::LLVMContext& context, llvm::Module& module) {
    switch (varType) {
        case VAR_TYPE_VOID:
            return llvm::Type::getVoidTy(context);
        case VAR_TYPE_BOOL:
            return llvm::Type::getInt1Ty(context);
        case VAR_TYPE_CHAR:
            return llvm::Type::getInt8Ty(context);
        case VAR_TYPE_SHORT:
            return llvm::Type::getInt16Ty(context);
        case VAR_TYPE_INT:
            return llvm::Type::getInt32Ty(context);
        case VAR_TYPE_LONG:
            return llvm::Type::getInt64Ty(context);
        case VAR_TYPE_FLOAT:
            return llvm::Type::getFloatTy(context);
        case VAR_TYPE_DOUBLE:
            return llvm::Type::getDoubleTy(context);
        default:
            return nullptr;
    }
}

llvm::Type* PointerTypeCodegen::getLLVMType(TypeManager& tm, llvm::LLVMContext& context, llvm::Module& module) {
    llvm::Type* pointeeLLVMType = tm.realize(pointeeIdx);
    if (!pointeeLLVMType)
        return nullptr;

    llvm::Type* result = pointeeLLVMType;
    for (int i = 0; i < level; ++i)
        result = llvm::PointerType::get(result, 0);

    return llvm::cast<llvm::PointerType>(result);
}

llvm::Type* QualifiedTypeCodegen::getLLVMType(TypeManager& tm, llvm::LLVMContext& context, llvm::Module& module) {
    return tm.realize(baseIdx);
}

llvm::Type* ArrayTypeCodegen::getLLVMType(TypeManager& tm, llvm::LLVMContext& context, llvm::Module& module) {
    llvm::Type* elemLLVMType = tm.realize(elementIdx);
    if (!elemLLVMType)
        return nullptr;
    return llvm::ArrayType::get(elemLLVMType, size);
}

llvm::Type* StructTypeCodegen::getLLVMType(TypeManager& tm, llvm::LLVMContext& context, llvm::Module& module) {
    llvm::StructType* existing = llvm::StructType::getTypeByName(module.getContext(), name);
    if (existing && !existing->isOpaque())
        return existing;

    llvm::StructType* llvmStruct = llvm::StructType::create(context, name);
    if (false == this->hasMembers()) {
        // If no members provided, return opaque struct type (for forward declaration)
        return llvmStruct;
    }

    std::vector<llvm::Type*> memberLLVMTypes;
    for (const MemberInfo& memberInfo : memberInfos) {
        llvm::Type* memberType = tm.realize(memberInfo.typeIdx);
        if (!memberType)
            break;
        memberLLVMTypes.push_back(memberType);
    }

    llvmStruct->setBody(memberLLVMTypes);
    return llvmStruct;
}

// ==================== StructMetadata ====================

// ==================== TypeManager ====================

TypeManager::TypeManager(llvm::LLVMContext& ctx, llvm::Module& mod) : context(ctx), module(mod) {}

TypeIdx TypeManager::registerType(const TypeKey& key, std::unique_ptr<TypeCodegen> node) {
    auto it = cache_.find(key);
    if (it != cache_.end())
        return it->second;
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

TypeIdx TypeManager::getQualifiedIdx(TypeIdx base, uint8_t qualifiers) {
    if (qualifiers == QUAL_NONE)
        return base;
    TypeKey key;
    key.kind = TypeKey::Qualified;
    key.baseIdx = base;
    key.qualifiers = qualifiers;
    return registerType(key, std::make_unique<QualifiedTypeCodegen>(base, qualifiers));
}

TypeIdx TypeManager::getArrayIdx(TypeIdx elem, std::vector<int> dims) {
    TypeIdx current = elem;
    for (auto it = dims.rbegin(); it != dims.rend(); ++it) {
        TypeKey key;
        key.kind = TypeKey::Array;
        key.elementIdx = current;
        key.size = *it;
        current = registerType(key, std::make_unique<ArrayTypeCodegen>(current, *it));
    }
    return current;
}

TypeIdx TypeManager::getStructIdx(const std::string& name, NStructDeclaration* members) {
    TypeKey key;
    key.kind = TypeKey::Struct;
    key.structName = name;
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        if (members != nullptr) {
            StructTypeCodegen* existing = static_cast<StructTypeCodegen*>(types_[it->second].get());
            if (false == existing->hasMembers()) {
                existing->setMembers(members);
            }
        }
        return it->second;
    }
    return registerType(key, std::make_unique<StructTypeCodegen>(name, members));
}

bool TypeManager::isConstQualified(TypeIdx idx) const {
    if (auto* q = dynamic_cast<const QualifiedTypeCodegen*>(get(idx)))
        return q->isConst();
    return false;
}

bool TypeManager::isVolatileQualified(TypeIdx idx) const {
    if (auto* q = dynamic_cast<const QualifiedTypeCodegen*>(get(idx)))
        return q->isVolatile();
    return false;
}

TypeIdx TypeManager::unqualify(TypeIdx idx) const {
    if (auto* q = dynamic_cast<const QualifiedTypeCodegen*>(get(idx)))
        return q->getBaseIdx();
    return idx;
}

bool TypeManager::isFloatingPointType(TypeIdx idx) const {
    const TypeCodegen* tc = get(idx);
    if (auto* ptc = dynamic_cast<const PrimitiveTypeCodegen*>(tc)) {
        VarType vt = ptc->getVarType();
        return vt == VAR_TYPE_FLOAT || vt == VAR_TYPE_DOUBLE;
    }
    return false;
}

ExprCodegenResult TypeManager::typeCast(llvm::Value* value, TypeIdx fromTypeIdx, TypeIdx toTypeIdx,
                                        llvm::IRBuilder<>& builder) {
    llvm::Type* fromType = realize(fromTypeIdx);
    llvm::Type* toType = realize(toTypeIdx);

    if (!value || !fromType || !toType) {
        return ExprCodegenResult("Type cast failed due to null value or type");
    }

    if (fromTypeIdx == toTypeIdx) {
        return ExprCodegenResult(value, toTypeIdx);
    }

    llvm::Value* result = nullptr;
    bool fromIsFloat = fromType && (fromType->isFloatTy() || fromType->isDoubleTy());
    bool toIsFloat = toType && (toType->isFloatTy() || toType->isDoubleTy());

    bool toIsInt = toType && toType->isIntegerTy();
    bool fromIsInt = fromType && fromType->isIntegerTy();

    // Bool to X
    if (fromType->isIntegerTy(1)) {
        if (toIsInt) {
            result = builder.CreateZExt(value, toType, "bool_to_int");
        } else if (toIsFloat) {
            result = builder.CreateUIToFP(value, toType, "bool_to_float");
        } else {
            return ExprCodegenResult("Unsupported cast from bool to " + getTypeName(toType));
        }
        return ExprCodegenResult(result, toTypeIdx);
    }

    // X to Bool
    if (toType->isIntegerTy(1)) {
        if (fromIsFloat) {
            result = builder.CreateFCmpONE(value, llvm::ConstantFP::get(fromType, 0.0), "to_bool");
        } else if (fromIsInt) {
            result = builder.CreateICmpNE(value, llvm::ConstantInt::get(fromType, 0), "to_bool");
        } else {
            return ExprCodegenResult("Unsupported cast to bool from " + getTypeName(fromType));
        }
        return ExprCodegenResult(result, toTypeIdx);
    }

    // Int to Int
    if (fromIsInt && toIsInt) {
        result = builder.CreateIntCast(value, toType, true, "int_cast");
        return ExprCodegenResult(result, toTypeIdx);
    }

    // Float to Int
    if (fromIsFloat && toIsInt) {
        result = builder.CreateFPToSI(value, toType, "float_to_int");
        return ExprCodegenResult(result, toTypeIdx);
    }

    // Int to Float
    if (fromIsInt && toIsFloat) {
        result = builder.CreateSIToFP(value, toType, "int_to_float");
        return ExprCodegenResult(result, toTypeIdx);
    }

    // Float to Float
    if (fromIsFloat && toIsFloat) {
        result = builder.CreateFPCast(value, toType, "float_cast");
        return ExprCodegenResult(result, toTypeIdx);
    }

    // Pointer casts
    if (fromType->isPointerTy() && toType->isPointerTy()) {
        result = builder.CreateBitCast(value, toType, "ptr_cast");
        return ExprCodegenResult(result, toTypeIdx);
    }

    // Int to Pointer
    if (fromIsInt && toType->isPointerTy()) {
        result = builder.CreateIntToPtr(value, toType, "int_to_ptr");
        return ExprCodegenResult(result, toTypeIdx);
    }

    // Pointer to Int
    if (fromType->isPointerTy() && toIsInt) {
        result = builder.CreatePtrToInt(value, toType, "ptr_to_int");
        return ExprCodegenResult(result, toTypeIdx);
    }

    return ExprCodegenResult("Unsupported type cast from " + getTypeName(fromType) + " to " + getTypeName(toType));
}

const TypeCodegen* TypeManager::get(TypeIdx idx) const {
    if (idx == InvalidTypeIdx || idx >= static_cast<TypeIdx>(types_.size()))
        return nullptr;
    return types_[idx].get();
}

llvm::Type* TypeManager::realize(TypeIdx idx) {
    if (idx == InvalidTypeIdx || idx >= static_cast<TypeIdx>(types_.size()))
        return nullptr;
    return types_[idx]->getLLVMType(*this, context, module);
}

// ==================== LLVM-level helpers ====================

TypeIdx TypeManager::getCommonTypeIdx(TypeIdx a, TypeIdx b) {
    if (a == b)
        return a;
    llvm::Type* ta = realize(a);
    llvm::Type* tb = realize(b);
    llvm::Type* common = getCommonType(ta, tb);
    if (common == ta)
        return a;
    if (common == tb)
        return b;
    // Fallback for float promotion (double)
    return (common && common->isDoubleTy()) ? getPrimitiveIdx(VAR_TYPE_DOUBLE) : getPrimitiveIdx(VAR_TYPE_FLOAT);
}

std::string TypeManager::getTypeName(llvm::Type* type) const {
    if (!type)
        return "null";
    if (type->isIntegerTy(1))
        return "bool";
    if (type->isIntegerTy(8))
        return "char";
    if (type->isIntegerTy(16))
        return "short";
    if (type->isIntegerTy(32))
        return "int";
    if (type->isIntegerTy(64))
        return "long";
    if (type->isFloatTy())
        return "float";
    if (type->isDoubleTy())
        return "double";
    if (type->isVoidTy())
        return "void";
    if (type->isPointerTy())
        return "ptr";
    if (type->isArrayTy()) {
        auto* arrayType = llvm::cast<llvm::ArrayType>(type);
        return "array[" + std::to_string(arrayType->getNumElements()) + "]";
    }
    if (auto* structType = llvm::dyn_cast<llvm::StructType>(type)) {
        std::string name;
        if (structType->hasName()) {
            return structType->getName().str();
        }
        return "anonymous_struct";
    }
    return "unknown";
}

llvm::Type* TypeManager::getCommonType(llvm::Type* type1, llvm::Type* type2) {
    if (!type1 || !type2)
        return nullptr;
    if (type1 == type2)
        return type1;

    if (type1->isDoubleTy() || type2->isDoubleTy()) {
        return llvm::Type::getDoubleTy(context);
    } else if (type1->isFloatTy() || type2->isFloatTy()) {
        return llvm::Type::getFloatTy(context);
    }

    auto getIntegerRank = [](llvm::Type* t) -> int {
        if (t->isIntegerTy(64))
            return 5;
        if (t->isIntegerTy(32))
            return 4;
        if (t->isIntegerTy(16))
            return 3;
        if (t->isIntegerTy(8))
            return 2;
        if (t->isIntegerTy(1))
            return 1;
        return 0;
    };
    return (getIntegerRank(type1) >= getIntegerRank(type2)) ? type1 : type2;
}
