#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <unordered_map>
#include <functional>
#include <cstdint>
#include <climits>

// Forward declarations for LLVM types
namespace llvm {
    class Type;
    class StructType;
    class PointerType;
    class ArrayType;
    class LLVMContext;
    class Module;
}

namespace toyc::ast {

class ASTContext;

enum VarType {
    VAR_TYPE_VOID = 0,
    VAR_TYPE_DEFINED = 1,
    VAR_TYPE_CHAR = 2,
    VAR_TYPE_BOOL = 3,
    VAR_TYPE_SHORT = 4,
    VAR_TYPE_INT = 5,
    VAR_TYPE_LONG = 6,
    VAR_TYPE_FLOAT = 7,
    VAR_TYPE_DOUBLE = 8,
    VAR_TYPE_PTR = 9,
    VAR_TYPE_STRUCT = 10,
    VAR_TYPE_ARRAY = 11
};

// Helper functions
bool isFloatingPointType(llvm::Type* type);
bool isIntegerType(llvm::Type* type);

// ==================== TypeIdx ====================

using TypeIdx = uint32_t;
static constexpr TypeIdx InvalidTypeIdx = UINT32_MAX;

// ==================== Forward Declarations ====================
class TypeManager;
class NDeclarator;

// ==================== NStructDeclaration ====================

class NStructDeclaration {
public:
    NStructDeclaration(TypeIdx typeIdx, NDeclarator* declarator)
        : typeIdx(typeIdx), declarator(declarator) {}
    ~NStructDeclaration();

    TypeIdx typeIdx;
    NDeclarator *declarator = nullptr;
    NStructDeclaration *next = nullptr;
};

// ==================== TypeCodegen ====================
// Abstract base for singleton type nodes stored in TypeManager.
// Each unique type has exactly one TypeCodegen instance.

class TypeCodegen {
public:
    virtual ~TypeCodegen() = default;
    virtual llvm::Type* getLLVMType(TypeManager& tm) = 0;
};

class PrimitiveTypeCodegen : public TypeCodegen {
public:
    explicit PrimitiveTypeCodegen(VarType vt) : varType(vt) {}
    llvm::Type* getLLVMType(TypeManager& tm) override;
    VarType getVarType() const { return varType; }
private:
    VarType varType;
};

class PointerTypeCodegen : public TypeCodegen {
public:
    PointerTypeCodegen(TypeIdx pointee, int level)
        : pointeeIdx(pointee), level(level) {}
    llvm::Type* getLLVMType(TypeManager& tm) override;
    TypeIdx getPointeeIdx() const { return pointeeIdx; }
    int getLevel() const { return level; }
private:
    TypeIdx pointeeIdx;
    int level;
};

class ArrayTypeCodegen : public TypeCodegen {
public:
    ArrayTypeCodegen(TypeIdx elem, std::vector<int> dims)
        : elementIdx(elem), dimensions(std::move(dims)) {}
    llvm::Type* getLLVMType(TypeManager& tm) override;
    TypeIdx getElementIdx() const { return elementIdx; }
    const std::vector<int>& getDimensions() const { return dimensions; }
private:
    TypeIdx elementIdx;
    std::vector<int> dimensions;
};

class StructTypeCodegen : public TypeCodegen {
public:
    StructTypeCodegen(std::string name, NStructDeclaration* members)
        : name(std::move(name)), members(members) {}
    ~StructTypeCodegen();
    llvm::Type* getLLVMType(TypeManager& tm) override;
    const std::string& getName() const { return name; }
    NStructDeclaration* getMembers() const { return members; }
    void setMembers(NStructDeclaration* m) { members = m; }
private:
    std::string name;
    NStructDeclaration* members; // owned by this node
};

// ==================== Struct Metadata ====================

struct StructMetadata {
    std::string name;
    std::map<std::string, int> memberIndexMap;
    std::vector<llvm::Type*> memberTypes;

    int getMemberIndex(const std::string& memberName) const {
        auto it = memberIndexMap.find(memberName);
        return (it != memberIndexMap.end()) ? it->second : -1;
    }

    llvm::Type* getMemberType(int index) const {
        if (index >= 0 && static_cast<size_t>(index) < memberTypes.size())
            return memberTypes[index];
        return nullptr;
    }
};

// ==================== TypeKey ====================

struct TypeKey {
    enum Kind { Primitive, Pointer, Array, Struct } kind;
    VarType varType = VAR_TYPE_VOID;       // Primitive only
    TypeIdx pointeeIdx = InvalidTypeIdx;   // Pointer only
    int level = 0;                         // Pointer only
    TypeIdx elementIdx = InvalidTypeIdx;   // Array only
    std::vector<int> dims;                 // Array only
    std::string structName;                // Struct only

    bool operator==(const TypeKey& o) const {
        if (kind != o.kind) return false;
        switch (kind) {
            case Primitive: return varType == o.varType;
            case Pointer:   return pointeeIdx == o.pointeeIdx && level == o.level;
            case Array:     return elementIdx == o.elementIdx && dims == o.dims;
            case Struct:    return structName == o.structName;
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
                for (int d : k.dims) h ^= std::hash<int>{}(d) << 2;
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

    // ==================== TypeIdx Factory (singleton per unique type) ====================
    TypeIdx getPrimitiveIdx(VarType vt);
    TypeIdx getPointerIdx(TypeIdx pointee, int level = 1);
    TypeIdx getArrayIdx(TypeIdx elem, std::vector<int> dims);
    TypeIdx getStructIdx(const std::string& name, NStructDeclaration* members);

    // ==================== TypeCodegen Access ====================
    const TypeCodegen* get(TypeIdx idx) const;

    // ==================== Realization: TypeIdx -> llvm::Type* ====================
    llvm::Type* realize(TypeIdx idx);

    // ==================== LLVM-level helpers (for codegen use) ====================
    llvm::Type* getPointeeType(llvm::Type* pointerType);
    const std::shared_ptr<StructMetadata> getStructMetadata(llvm::StructType* structType) const;
    std::string getTypeName(llvm::Type* type) const;
    llvm::Type* getCommonType(llvm::Type* type1, llvm::Type* type2);

    // ==================== Accessors for TypeCodegen::getLLVMType implementations ====================
    llvm::LLVMContext& getLLVMContext() { return context; }
    llvm::Module& getModule() { return module; }
    llvm::Type* getBasicType(VarType vt);
    llvm::StructType* getStructType(const std::string& name);

    // TypeCodegen calls these to register metadata after building LLVM types
    void registerPointerMetadata(llvm::Type* ptrType, llvm::Type* pointeeType);
    void registerStructMetadata(llvm::StructType* structType, std::shared_ptr<StructMetadata> meta);

private:
    TypeIdx registerType(const TypeKey& key, std::unique_ptr<TypeCodegen> node);

    llvm::LLVMContext& context;
    llvm::Module& module;

    std::vector<std::unique_ptr<TypeCodegen>> types_;
    std::unordered_map<TypeKey, TypeIdx, TypeKeyHash> cache_;

    // LLVM-level metadata caches (used by codegen helpers)
    std::map<llvm::Type*, llvm::Type*> pointerMetadataMap;
    std::map<llvm::StructType*, std::shared_ptr<StructMetadata>> structMetadataByType;
};

} // namespace toyc::ast
