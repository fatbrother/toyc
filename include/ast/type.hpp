#pragma once

#include <string>
#include <vector>
#include <memory>

// Forward declarations for LLVM types
namespace llvm {
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

class NType {
public:
    NType(VarType type) : type(type) {}
    virtual ~NType() = default;

    virtual std::string getType() const { return "Type"; }
    virtual TypeCodegenResult getLLVMType(ASTContext &context);

    virtual bool isArray() const { return type == VAR_TYPE_ARRAY; }
    virtual bool isPointer() const { return type == VAR_TYPE_PTR; }
    virtual bool isStruct() const { return type == VAR_TYPE_STRUCT; }
    virtual NTypePtr getElementType() const { return nullptr; }
    virtual NTypePtr getAddrType();
    virtual VarType getVarType() const { return type; }
    virtual std::string getName() const { return varTypeToString(type); }

private:
    VarType type;
};

class NPointerType : public NType {
public:
    NPointerType(NTypePtr pointeeType, int level = 1);
    NPointerType(VarType pointeeType, int level = 1);

    virtual std::string getType() const override { return "PointerType"; }
    virtual TypeCodegenResult getLLVMType(ASTContext &context) override;
    virtual NTypePtr getElementType() const override;
    virtual NTypePtr getAddrType() override;
    virtual std::string getName() const override {
        return pointeeType->getName() + std::string(level, '*');
    }

private:
    NTypePtr pointeeType;
    int level;
};

class NArrayType : public NType {
public:
    NArrayType(NTypePtr elementType, const std::vector<int> &dimensions);
    NArrayType(NType *elementType, const std::vector<int> &dimensions);

    virtual std::string getType() const override { return "ArrayType"; }
    virtual TypeCodegenResult getLLVMType(ASTContext &context) override;
    virtual NTypePtr getElementType() const override;
    virtual std::string getName() const override {
        std::string name = elementType->getName();
        for (int dim : arrayDimensions) {
            name += "[" + std::to_string(dim) + "]";
        }
        return name;
    }

    const std::vector<int>& getArrayDimensions() const;
    int getTotalArraySize() const;

private:
    NTypePtr elementType;
    std::vector<int> arrayDimensions;
};

class NDeclarator;
class NStructDeclaration {
public:
    NStructDeclaration(NType *type, NDeclarator *declarator)
        : type(type), declarator(declarator) {}
    ~NStructDeclaration();

    NTypePtr type;
    NDeclarator *declarator = nullptr;
    NStructDeclaration *next = nullptr;
};

class NStructType : public NType {
public:
    NStructType(const std::string &name, NStructDeclaration *members);
    virtual ~NStructType();

    virtual std::string getType() const override { return "StructType"; }
    virtual TypeCodegenResult getLLVMType(ASTContext &context) override;
    virtual std::string getName() const override { return name; }

    int getMemberIndex(const std::string &memberName) const;
    NTypePtr getMemberType(const std::string &memberName) const;
    NStructDeclaration* getMembers() const { return members; }

private:
    std::string name;
    NStructDeclaration *members = nullptr;
    llvm::StructType *llvmStructType = nullptr;
};

} // namespace toyc::ast
