#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "ast/codegen_result.hpp"
#include "define.hpp"

namespace toyc::ast {

class ASTContext;

// ==================== Forward Declarations ====================
class TypeManager;
class NDeclarator;

// ==================== NStructDeclaration ====================

class NStructDeclaration {
public:
    NStructDeclaration(TypeIdx typeIdx, NDeclarator* declarator) : typeIdx(typeIdx), declarator(declarator) {}
    ~NStructDeclaration();

    TypeIdx typeIdx;
    NDeclarator* declarator = nullptr;
    NStructDeclaration* next = nullptr;
};

// ==================== TypeCodegen ====================
// Abstract base for singleton type nodes stored in TypeManager.
// Each unique type has exactly one TypeCodegen instance.

class TypeCodegen {
public:
    virtual ~TypeCodegen() = default;
    virtual llvm::Type* getLLVMType(TypeManager& tm, llvm::LLVMContext& context, llvm::Module& module) = 0;
};

class PrimitiveTypeCodegen : public TypeCodegen {
public:
    explicit PrimitiveTypeCodegen(VarType vt) : varType(vt) {}
    llvm::Type* getLLVMType(TypeManager& tm, llvm::LLVMContext& context, llvm::Module& module) override;
    VarType getVarType() const { return varType; }

private:
    VarType varType;
};

class PointerTypeCodegen : public TypeCodegen {
public:
    PointerTypeCodegen(TypeIdx pointee, int level) : pointeeIdx(pointee), level(level) {}
    llvm::Type* getLLVMType(TypeManager& tm, llvm::LLVMContext& context, llvm::Module& module) override;
    TypeIdx getPointeeIdx() const { return pointeeIdx; }
    int getLevel() const { return level; }

private:
    TypeIdx pointeeIdx;
    int level;
};

class ArrayTypeCodegen : public TypeCodegen {
public:
    ArrayTypeCodegen(TypeIdx elem, int size) : elementIdx(elem), size(size) {}
    llvm::Type* getLLVMType(TypeManager& tm, llvm::LLVMContext& context, llvm::Module& module) override;
    TypeIdx getElementIdx() const { return elementIdx; }
    int getSize() const { return size; }

private:
    TypeIdx elementIdx;
    int size;
};

class StructTypeCodegen : public TypeCodegen {
public:
    struct MemberInfo {
        std::string name;
        TypeIdx typeIdx;
    };

    StructTypeCodegen(std::string name, NStructDeclaration* members);
    llvm::Type* getLLVMType(TypeManager& tm, llvm::LLVMContext& context, llvm::Module& module) override;
    const std::string& getName() const { return name; }
    bool hasMembers() const { return !memberInfos.empty(); }
    void setMembers(NStructDeclaration* m);
    int getMemberIndex(const std::string& memberName) const;
    TypeIdx getMemberTypeIdx(int index) const;

private:
    std::string name;
    std::vector<MemberInfo> memberInfos;
};

// ==================== TypeKey ====================

struct TypeKey {
    enum Kind { Primitive, Pointer, Array, Struct } kind;
    VarType varType = VAR_TYPE_VOID;      // Primitive only
    TypeIdx pointeeIdx = InvalidTypeIdx;  // Pointer only
    int level = 0;                        // Pointer only
    TypeIdx elementIdx = InvalidTypeIdx;  // Array only
    int size = 0;                         // Array only
    std::string structName;               // Struct only

    bool operator==(const TypeKey& o) const {
        if (kind != o.kind)
            return false;
        switch (kind) {
            case Primitive:
                return varType == o.varType;
            case Pointer:
                return pointeeIdx == o.pointeeIdx && level == o.level;
            case Array:
                return elementIdx == o.elementIdx && size == o.size;
            case Struct:
                return structName == o.structName;
        }
        return false;
    }
};

struct TypeKeyHash {
    size_t operator()(const TypeKey& k) const {
        size_t h = std::hash<int>{}(k.kind);
        switch (k.kind) {
            case TypeKey::Primitive:
                h ^= std::hash<int>{}(k.varType) << 1;
                break;
            case TypeKey::Pointer:
                h ^= std::hash<uint32_t>{}(k.pointeeIdx) << 1;
                h ^= std::hash<int>{}(k.level) << 2;
                break;
            case TypeKey::Array:
                h ^= std::hash<uint32_t>{}(k.elementIdx) << 1;
                h ^= std::hash<int>{}(k.size) << 2;
                break;
            case TypeKey::Struct:
                h ^= std::hash<std::string>{}(k.structName) << 1;
                break;
        }
        return h;
    }
};

// ==================== TypeManager ====================

class TypeManager {
public:
    TypeManager(llvm::LLVMContext& ctx, llvm::Module& mod);
    ~TypeManager() = default;
    TypeManager(const TypeManager&) = delete;
    TypeManager& operator=(const TypeManager&) = delete;

    // ==================== TypeIdx Factory ====================
    TypeIdx getPrimitiveIdx(VarType vt);
    TypeIdx getPointerIdx(TypeIdx pointee, int level = 1);
    TypeIdx getArrayIdx(TypeIdx elem, std::vector<int> dims);
    TypeIdx getStructIdx(const std::string& name, NStructDeclaration* members);

    // ==================== Type Info Access ====================
    bool isFloatingPointType(TypeIdx idx) const;
    ExprCodegenResult typeCast(llvm::Value* value, TypeIdx fromTypeIdx, TypeIdx toTypeIdx, llvm::IRBuilder<>& builder);

    // ==================== TypeCodegen Access ====================
    const TypeCodegen* get(TypeIdx idx) const;

    // ==================== Realization: TypeIdx -> llvm::Type* ====================
    llvm::Type* realize(TypeIdx idx);

    // ==================== TypeIdx-level helpers ====================
    TypeIdx getCommonTypeIdx(TypeIdx a, TypeIdx b);

    // ==================== LLVM-level helpers (for codegen use) ====================
    std::string getTypeName(llvm::Type* type) const;
    llvm::Type* getCommonType(llvm::Type* type1, llvm::Type* type2);

private:
    TypeIdx registerType(const TypeKey& key, std::unique_ptr<TypeCodegen> node);

    llvm::LLVMContext& context;
    llvm::Module& module;

    std::vector<std::unique_ptr<TypeCodegen>> types_;
    std::unordered_map<TypeKey, TypeIdx, TypeKeyHash> cache_;
};

}  // namespace toyc::ast
