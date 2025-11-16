#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>

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

// ==================== Forward Declarations ====================
class TypeManager;
class NDeclarator;
class NStructDeclaration;

// ==================== TypeDescriptor ====================

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

class NStructDeclaration {
public:
    NStructDeclaration(TypeDescriptor* type, NDeclarator* declarator)
        : type(type), declarator(declarator) {}
    ~NStructDeclaration();

    TypeDescriptor *type;
    NDeclarator *declarator = nullptr;
    NStructDeclaration *next = nullptr;
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

// ==================== Array Metadata ====================

struct ArrayMetadata {
    std::vector<int> dimensions;
    llvm::Type* elementType;

    int getTotalSize() const {
        int total = 1;
        for (int dim : dimensions) {
            total *= dim;
        }
        return total;
    }
};

// ==================== Struct Metadata ====================

struct StructMetadata {
    std::string name;
    std::map<std::string, int> memberIndexMap;
    std::vector<llvm::Type*> memberTypes;
    NStructDeclaration* members = nullptr;

    ~StructMetadata();

    int getMemberIndex(const std::string& memberName) const {
        auto it = memberIndexMap.find(memberName);
        return (it != memberIndexMap.end()) ? it->second : -1;
    }

    llvm::Type* getMemberType(int index) const {
        if (index >= 0 && static_cast<size_t>(index) < memberTypes.size()) {
            return memberTypes[index];
        }
        return nullptr;
    }
};

// ==================== TypeManager ====================

class TypeManager {
public:
    TypeManager(llvm::LLVMContext& ctx, llvm::Module& mod);
    ~TypeManager() = default;
    TypeManager(const TypeManager&) = delete;
    TypeManager& operator=(const TypeManager&) = delete;

    // ==================== Basic Types ====================
    llvm::Type* getVoidType();
    llvm::Type* getBoolType();
    llvm::Type* getCharType();
    llvm::Type* getShortType();
    llvm::Type* getIntType();
    llvm::Type* getLongType();
    llvm::Type* getFloatType();
    llvm::Type* getDoubleType();
    llvm::Type* getBasicType(VarType type);

    // =================== Pointer ====================
    llvm::PointerType* getPointerType(llvm::Type* pointeeType);
    llvm::PointerType* getPointerType(llvm::Type* pointeeType, int level);
    llvm::Type* getPointeeType(llvm::Type* pointerType);

    // ==================== Array =====================
    llvm::ArrayType* getArrayType(llvm::Type* elementType, const std::vector<int>& dimensions);
    llvm::Type* getArrayElementType(llvm::Type* arrayType);
    const ArrayMetadata* getArrayMetadata(llvm::Type* arrayType) const;

    // ==================== Struct ====================
    llvm::StructType* createStructType(const std::string& name, NStructDeclaration* members, ASTContext& context);
    llvm::StructType* getStructType(const std::string& name);
    const std::shared_ptr<StructMetadata> getStructMetadata(llvm::StructType* structType) const;

    // ==================== TypeDescriptor Realization ====================
    llvm::Type* realize(const TypeDescriptor* descriptor, ASTContext& context);

    // ==================== Helper Functions ====================
    std::string getTypeName(llvm::Type* type) const;
    llvm::Type* getCommonType(llvm::Type* type1, llvm::Type* type2);

private:
    llvm::LLVMContext& context;
    llvm::Module& module;

    std::map<llvm::Type*, llvm::Type*> pointerMetadataMap;  // pointer type -> pointee type
    std::map<llvm::Type*, ArrayMetadata> arrayMetadataMap;
    std::map<llvm::StructType*, std::shared_ptr<StructMetadata>> structMetadataByType;
};

} // namespace toyc::ast
