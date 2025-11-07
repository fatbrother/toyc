#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>

// Forward declarations for LLVM types
namespace llvm {
    class Type;
    class StructType;
}

namespace toyc::ast {

class ASTContext;
class TypeValue;
template<typename T> class CodegenResult;
using TypeCodegenResult = CodegenResult<TypeValue>;
class NType;
using NTypePtr = std::shared_ptr<NType>;

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
std::string varTypeToString(VarType type);
bool isFloatingPointType(VarType type);
bool isIntegerType(VarType type);

// ==================== Forward Declarations ====================
class TypeFactory;
class NDeclarator;
class NStructDeclaration;

// ==================== TypeDescriptor (用於 Parser 階段) ====================

struct TypeDescriptor {
    enum Kind {
        Primitive,
        Pointer,
        Array,
        Struct
    };

    virtual ~TypeDescriptor() = default;
    virtual Kind getKind() const = 0;
};

using TypeDescriptorPtr = std::unique_ptr<TypeDescriptor>;

struct PrimitiveTypeDescriptor : public TypeDescriptor {
    VarType varType;

    explicit PrimitiveTypeDescriptor(VarType varType)
        : varType(varType) {}

    Kind getKind() const override { return Primitive; }
};

struct PointerTypeDescriptor : public TypeDescriptor {
    TypeDescriptorPtr pointeeDesc;
    int level;

    PointerTypeDescriptor(TypeDescriptorPtr pointee, int level = 1)
        : pointeeDesc(std::move(pointee)), level(level) {}

    Kind getKind() const override { return Pointer; }
};

struct ArrayTypeDescriptor : public TypeDescriptor {
    TypeDescriptorPtr elementDesc;
    std::vector<int> dimensions;

    ArrayTypeDescriptor(TypeDescriptorPtr element, std::vector<int> dims)
        : elementDesc(std::move(element)), dimensions(std::move(dims)) {}

    Kind getKind() const override { return Array; }
};

struct StructTypeDescriptor : public TypeDescriptor {
    std::string name;
    NStructDeclaration* members;

    StructTypeDescriptor(std::string name, NStructDeclaration* members);
    ~StructTypeDescriptor();

    Kind getKind() const override { return Struct; }
};

// ==================== TypeDescriptor Helper Functions ====================

inline TypeDescriptorPtr makePrimitiveDesc(VarType type) {
    return std::make_unique<PrimitiveTypeDescriptor>(type);
}

inline TypeDescriptorPtr makeBoolDesc() {
    return makePrimitiveDesc(VAR_TYPE_BOOL);
}

inline TypeDescriptorPtr makeCharDesc() {
    return makePrimitiveDesc(VAR_TYPE_CHAR);
}

inline TypeDescriptorPtr makeShortDesc() {
    return makePrimitiveDesc(VAR_TYPE_SHORT);
}

inline TypeDescriptorPtr makeIntDesc() {
    return makePrimitiveDesc(VAR_TYPE_INT);
}

inline TypeDescriptorPtr makeLongDesc() {
    return makePrimitiveDesc(VAR_TYPE_LONG);
}

inline TypeDescriptorPtr makeFloatDesc() {
    return makePrimitiveDesc(VAR_TYPE_FLOAT);
}

inline TypeDescriptorPtr makeDoubleDesc() {
    return makePrimitiveDesc(VAR_TYPE_DOUBLE);
}

inline TypeDescriptorPtr makeVoidDesc() {
    return makePrimitiveDesc(VAR_TYPE_VOID);
}

inline TypeDescriptorPtr makePointerDesc(TypeDescriptorPtr pointee, int level = 1) {
    return std::make_unique<PointerTypeDescriptor>(std::move(pointee), level);
}

inline TypeDescriptorPtr makeArrayDesc(TypeDescriptorPtr element, std::vector<int> dims) {
    return std::make_unique<ArrayTypeDescriptor>(std::move(element), std::move(dims));
}

inline TypeDescriptorPtr makeStructDesc(const std::string& name, NStructDeclaration* members) {
    return std::make_unique<StructTypeDescriptor>(name, members);
}

// ==================== NType Classes ====================

class NType : public std::enable_shared_from_this<NType> {
public:
    virtual ~NType() = default;

    virtual std::string getType() const { return "Type"; }
    virtual TypeCodegenResult getLLVMType(ASTContext &context);

    virtual bool isArray() const { return type == VAR_TYPE_ARRAY; }
    virtual bool isPointer() const { return type == VAR_TYPE_PTR; }
    virtual bool isStruct() const { return type == VAR_TYPE_STRUCT; }
    virtual NTypePtr getElementType(ASTContext& context) const { return nullptr; }
    virtual NTypePtr getAddrType(ASTContext& context);
    virtual VarType getVarType() const { return type; }
    virtual std::string getName() const { return varTypeToString(type); }

protected:
    NType(VarType type) : type(type), cachedLLVMType(nullptr) {}
    virtual llvm::Type* generateLLVMType(ASTContext &context);

private:
    VarType type;
    mutable llvm::Type* cachedLLVMType;

    friend class TypeFactory;
};

class NPointerType : public NType {
public:
    virtual std::string getType() const override { return "PointerType"; }
    virtual NTypePtr getElementType(ASTContext& context) const override;
    virtual NTypePtr getAddrType(ASTContext& context) override;
    virtual std::string getName() const override {
        return pointeeType->getName() + std::string(level, '*');
    }

protected:
    NPointerType(NTypePtr pointeeType, int level = 1);

    virtual llvm::Type* generateLLVMType(ASTContext &context) override;

private:
    NTypePtr pointeeType;
    int level;

    friend class TypeFactory;
};

class NArrayType : public NType {
public:
    virtual std::string getType() const override { return "ArrayType"; }
    virtual NTypePtr getElementType(ASTContext& context) const override;
    virtual std::string getName() const override {
        std::string name = elementType->getName();
        for (int dim : arrayDimensions) {
            name += "[" + std::to_string(dim) + "]";
        }
        return name;
    }

    const std::vector<int>& getArrayDimensions() const;
    int getTotalArraySize() const;

protected:
    NArrayType(NTypePtr elementType, const std::vector<int> &dimensions);

    virtual llvm::Type* generateLLVMType(ASTContext &context) override;

private:
    NTypePtr elementType;
    std::vector<int> arrayDimensions;

    friend class TypeFactory;
};

class NDeclarator;
class NStructDeclaration {
public:
    NStructDeclaration(TypeDescriptor* type, NDeclarator* declarator)
        : type(type), declarator(declarator) {}
    ~NStructDeclaration();

    TypeDescriptor *type;
    NDeclarator *declarator = nullptr;
    NStructDeclaration *next = nullptr;
};

class NStructType : public NType {
public:
    virtual ~NStructType();

    virtual std::string getType() const override { return "StructType"; }
    virtual std::string getName() const override { return name; }

    int getMemberIndex(const std::string &memberName) const;
    NTypePtr getMemberType(const std::string &memberName, ASTContext& context) const;
    NStructDeclaration* getMembers() const { return members; }

protected:
    NStructType(const std::string &name, NStructDeclaration *members);

    virtual llvm::Type* generateLLVMType(ASTContext &context) override;

private:
    std::string name;
    NStructDeclaration *members = nullptr;

    friend class TypeFactory;
};

// ==================== TypeFactory (集中管理所有 Type 創建) ====================

class TypeFactory {
public:
    TypeFactory();
    ~TypeFactory() = default;
    TypeFactory(const TypeFactory&) = delete;
    TypeFactory& operator=(const TypeFactory&) = delete;

    // ==================== Basic Types ====================
    NTypePtr getVoidType() { return voidType; }
    NTypePtr getBoolType() { return boolType; }
    NTypePtr getCharType() { return charType; }
    NTypePtr getShortType() { return shortType; }
    NTypePtr getIntType() { return intType; }
    NTypePtr getLongType() { return longType; }
    NTypePtr getFloatType() { return floatType; }
    NTypePtr getDoubleType() { return doubleType; }
    NTypePtr getBasicType(VarType type);

    // =================== Pointer ====================
    NTypePtr getPointerType(NTypePtr pointeeType);
    NTypePtr getPointerType(NTypePtr pointeeType, int level);

    // ==================== Array =====================
    NTypePtr getArrayType(NTypePtr elementType, const std::vector<int>& dimensions);

    // ==================== Struct ====================
    NTypePtr createStructType(const std::string& name, NStructDeclaration* members);
    void registerStructType(const std::string& name, NTypePtr structType);
    NTypePtr getStructType(const std::string& name, NStructDeclaration* members = nullptr);
    bool hasStructType(const std::string& name) const;

    // ==================== TypeDescriptor Realization ====================
    NTypePtr realize(const TypeDescriptor* descriptor);

    // ==================== Typedef ====================
    // void registerTypedef(const std::string& name, NTypePtr underlyingType);
    // NTypePtr getTypedefType(const std::string& name);

private:
    NTypePtr voidType;
    NTypePtr boolType;
    NTypePtr charType;
    NTypePtr shortType;
    NTypePtr intType;
    NTypePtr longType;
    NTypePtr floatType;
    NTypePtr doubleType;

    std::map<size_t, NTypePtr> pointerTypeCache;
    struct ArrayKey {
        size_t elementTypeHash;
        std::vector<int> dimensions;

        bool operator<(const ArrayKey& other) const {
            if (elementTypeHash != other.elementTypeHash)
                return elementTypeHash < other.elementTypeHash;
            return dimensions < other.dimensions;
        }
    };
    std::map<ArrayKey, NTypePtr> arrayTypeCache;
    std::map<std::string, NTypePtr> structTypeRegistry;

    size_t computeTypeHash(NTypePtr type) const;
};

} // namespace toyc::ast
