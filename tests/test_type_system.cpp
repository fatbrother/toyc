#include <gtest/gtest.h>
#include "ast/type.hpp"
#include "ast/node.hpp"

using namespace toyc::ast;

class TypeSystemTest : public ::testing::Test {
protected:
    ASTContext context;

    void SetUp() override {
        // 每個測試前的設定
    }

    void TearDown() override {
        // 每個測試後的清理
    }
};

// ==================== Primitive Type Tests ====================

TEST_F(TypeSystemTest, PrimitiveTypeCreation) {
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);
    ASSERT_NE(intType, nullptr);
    EXPECT_EQ(intType->getVarType(), VAR_TYPE_INT);
    EXPECT_EQ(intType->getName(), "int");
}

TEST_F(TypeSystemTest, PrimitiveTypeSingleton) {
    // 相同的基本型別應該返回同一個實例（快取）
    NTypePtr int1 = context.typeFactory->getBasicType(VAR_TYPE_INT);
    NTypePtr int2 = context.typeFactory->getBasicType(VAR_TYPE_INT);

    EXPECT_EQ(int1.get(), int2.get()) << "Same primitive type should return same instance";
}

TEST_F(TypeSystemTest, AllPrimitiveTypes) {
    std::vector<VarType> types = {
        VAR_TYPE_VOID,
        VAR_TYPE_BOOL,
        VAR_TYPE_CHAR,
        VAR_TYPE_SHORT,
        VAR_TYPE_INT,
        VAR_TYPE_LONG,
        VAR_TYPE_FLOAT,
        VAR_TYPE_DOUBLE
    };

    for (auto varType : types) {
        NTypePtr type = context.typeFactory->getBasicType(varType);
        ASSERT_NE(type, nullptr) << "Failed to create type: " << varTypeToString(varType);
        EXPECT_EQ(type->getVarType(), varType);
    }
}

TEST_F(TypeSystemTest, PrimitiveTypeLLVMGeneration) {
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);
    TypeCodegenResult result = intType->getLLVMType(context);

    ASSERT_TRUE(result.isSuccess()) << "LLVM type generation failed: " << result.getErrorMessage();
    ASSERT_NE(result.getLLVMType(), nullptr);
    EXPECT_TRUE(result.getLLVMType()->isIntegerTy(32));
}

TEST_F(TypeSystemTest, PrimitiveTypeLLVMCache) {
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);

    // 第一次生成
    TypeCodegenResult result1 = intType->getLLVMType(context);
    llvm::Type* llvmType1 = result1.getLLVMType();

    // 第二次應該使用快取
    TypeCodegenResult result2 = intType->getLLVMType(context);
    llvm::Type* llvmType2 = result2.getLLVMType();

    EXPECT_EQ(llvmType1, llvmType2) << "LLVM type should be cached";
}

// ==================== Pointer Type Tests ====================

TEST_F(TypeSystemTest, PointerTypeCreation) {
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);
    NTypePtr ptrType = context.typeFactory->getPointerType(intType);

    ASSERT_NE(ptrType, nullptr);
    EXPECT_TRUE(ptrType->isPointer());
    EXPECT_EQ(ptrType->getName(), "int*");
}

TEST_F(TypeSystemTest, PointerTypeSingleton) {
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);
    NTypePtr ptr1 = context.typeFactory->getPointerType(intType);
    NTypePtr ptr2 = context.typeFactory->getPointerType(intType);

    EXPECT_EQ(ptr1.get(), ptr2.get()) << "Same pointer type should return same instance";
}

TEST_F(TypeSystemTest, MultiLevelPointer) {
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);
    NTypePtr intPtrPtr = context.typeFactory->getPointerType(intType, 2);

    ASSERT_NE(intPtrPtr, nullptr);
    EXPECT_EQ(intPtrPtr->getName(), "int**");
}

TEST_F(TypeSystemTest, PointerGetElementType) {
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);
    NTypePtr intPtr = context.typeFactory->getPointerType(intType);

    NTypePtr elemType = intPtr->getElementType(context);
    ASSERT_NE(elemType, nullptr);
    EXPECT_EQ(elemType.get(), intType.get());
}

TEST_F(TypeSystemTest, MultiLevelPointerGetElementType) {
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);
    NTypePtr intPtrPtr = context.typeFactory->getPointerType(intType, 2);

    // int** -> int*
    NTypePtr intPtr = intPtrPtr->getElementType(context);
    ASSERT_NE(intPtr, nullptr);
    EXPECT_EQ(intPtr->getName(), "int*");

    // int* -> int
    NTypePtr finalType = intPtr->getElementType(context);
    ASSERT_NE(finalType, nullptr);
    EXPECT_EQ(finalType.get(), intType.get());
}

TEST_F(TypeSystemTest, PointerGetAddrType) {
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);
    NTypePtr intPtr = intType->getAddrType(context);

    ASSERT_NE(intPtr, nullptr);
    EXPECT_TRUE(intPtr->isPointer());
    EXPECT_EQ(intPtr->getName(), "int*");
}

TEST_F(TypeSystemTest, PointerLLVMGeneration) {
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);
    NTypePtr ptrType = context.typeFactory->getPointerType(intType);

    TypeCodegenResult result = ptrType->getLLVMType(context);
    ASSERT_TRUE(result.isSuccess());
    ASSERT_NE(result.getLLVMType(), nullptr);
    EXPECT_TRUE(result.getLLVMType()->isPointerTy());
}

// ==================== Array Type Tests ====================

TEST_F(TypeSystemTest, ArrayTypeCreation) {
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);
    std::vector<int> dims = {10};
    NTypePtr arrayType = context.typeFactory->getArrayType(intType, dims);

    ASSERT_NE(arrayType, nullptr);
    EXPECT_TRUE(arrayType->isArray());
    EXPECT_EQ(arrayType->getName(), "int[10]");
}

TEST_F(TypeSystemTest, ArrayTypeSingleton) {
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);
    std::vector<int> dims = {10};

    NTypePtr arr1 = context.typeFactory->getArrayType(intType, dims);
    NTypePtr arr2 = context.typeFactory->getArrayType(intType, dims);

    EXPECT_EQ(arr1.get(), arr2.get()) << "Same array type should return same instance";
}

TEST_F(TypeSystemTest, MultiDimensionalArray) {
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);
    std::vector<int> dims = {10, 20};
    NTypePtr arrayType = context.typeFactory->getArrayType(intType, dims);

    ASSERT_NE(arrayType, nullptr);
    EXPECT_EQ(arrayType->getName(), "int[10][20]");
}

TEST_F(TypeSystemTest, ArrayGetElementType) {
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);
    std::vector<int> dims = {10};
    NTypePtr arrayType = context.typeFactory->getArrayType(intType, dims);

    NTypePtr elemType = arrayType->getElementType(context);
    ASSERT_NE(elemType, nullptr);
    EXPECT_EQ(elemType.get(), intType.get());
}

TEST_F(TypeSystemTest, MultiDimensionalArrayGetElementType) {
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);
    std::vector<int> dims = {10, 20};
    NTypePtr arrayType = context.typeFactory->getArrayType(intType, dims);

    // int[10][20] -> int[20]
    NTypePtr innerArray = arrayType->getElementType(context);
    ASSERT_NE(innerArray, nullptr);
    EXPECT_TRUE(innerArray->isArray());
    EXPECT_EQ(innerArray->getName(), "int[20]");

    // int[20] -> int
    NTypePtr finalType = innerArray->getElementType(context);
    ASSERT_NE(finalType, nullptr);
    EXPECT_EQ(finalType.get(), intType.get());
}

TEST_F(TypeSystemTest, ArrayLLVMGeneration) {
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);
    std::vector<int> dims = {10};
    NTypePtr arrayType = context.typeFactory->getArrayType(intType, dims);

    TypeCodegenResult result = arrayType->getLLVMType(context);
    ASSERT_TRUE(result.isSuccess());
    ASSERT_NE(result.getLLVMType(), nullptr);
    EXPECT_TRUE(result.getLLVMType()->isArrayTy());
}

TEST_F(TypeSystemTest, ArrayTotalSize) {
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);
    std::vector<int> dims = {10, 20, 30};
    NTypePtr arrayType = context.typeFactory->getArrayType(intType, dims);

    auto arrayTypeCast = std::static_pointer_cast<NArrayType>(arrayType);
    EXPECT_EQ(arrayTypeCast->getTotalArraySize(), 10 * 20 * 30);
}

// ==================== Struct Type Tests ====================

TEST_F(TypeSystemTest, StructTypeCreation) {
    NTypePtr structType = context.typeFactory->getStructType("MyStruct", nullptr);

    ASSERT_NE(structType, nullptr);
    EXPECT_TRUE(structType->isStruct());
    EXPECT_EQ(structType->getName(), "MyStruct");
}

TEST_F(TypeSystemTest, StructTypeSingleton) {
    NTypePtr struct1 = context.typeFactory->getStructType("MyStruct", nullptr);
    NTypePtr struct2 = context.typeFactory->getStructType("MyStruct", nullptr);

    EXPECT_EQ(struct1.get(), struct2.get()) << "Same struct name should return same instance";
}

TEST_F(TypeSystemTest, DifferentStructTypes) {
    NTypePtr struct1 = context.typeFactory->getStructType("Struct1", nullptr);
    NTypePtr struct2 = context.typeFactory->getStructType("Struct2", nullptr);

    EXPECT_NE(struct1.get(), struct2.get()) << "Different struct names should return different instances";
}

// ==================== Type Descriptor Tests ====================

TEST_F(TypeSystemTest, PrimitiveTypeDescriptorRealize) {
    auto desc = makePrimitiveDesc(VAR_TYPE_INT);
    NTypePtr type = context.typeFactory->realize(desc.get());

    ASSERT_NE(type, nullptr);
    EXPECT_EQ(type->getVarType(), VAR_TYPE_INT);
}

TEST_F(TypeSystemTest, PointerTypeDescriptorRealize) {
    auto intDesc = makePrimitiveDesc(VAR_TYPE_INT);
    auto ptrDesc = makePointerDesc(std::move(intDesc), 1);
    NTypePtr type = context.typeFactory->realize(ptrDesc.get());

    ASSERT_NE(type, nullptr);
    EXPECT_TRUE(type->isPointer());
    EXPECT_EQ(type->getName(), "int*");
}

TEST_F(TypeSystemTest, ArrayTypeDescriptorRealize) {
    auto intDesc = makePrimitiveDesc(VAR_TYPE_INT);
    std::vector<int> dims = {10};
    auto arrayDesc = makeArrayDesc(std::move(intDesc), dims);
    NTypePtr type = context.typeFactory->realize(arrayDesc.get());

    ASSERT_NE(type, nullptr);
    EXPECT_TRUE(type->isArray());
    EXPECT_EQ(type->getName(), "int[10]");
}

TEST_F(TypeSystemTest, StructTypeDescriptorRealize) {
    auto structDesc = makeStructDesc("MyStruct", nullptr);
    NTypePtr type = context.typeFactory->realize(structDesc.get());

    ASSERT_NE(type, nullptr);
    EXPECT_TRUE(type->isStruct());
    EXPECT_EQ(type->getName(), "MyStruct");
}

// ==================== Complex Type Tests ====================

TEST_F(TypeSystemTest, PointerToArray) {
    // int (*)[10] - pointer to array of 10 ints
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);
    std::vector<int> dims = {10};
    NTypePtr arrayType = context.typeFactory->getArrayType(intType, dims);
    NTypePtr ptrToArray = context.typeFactory->getPointerType(arrayType);

    ASSERT_NE(ptrToArray, nullptr);
    EXPECT_TRUE(ptrToArray->isPointer());

    NTypePtr elemType = ptrToArray->getElementType(context);
    EXPECT_TRUE(elemType->isArray());
}

TEST_F(TypeSystemTest, ArrayOfPointers) {
    // int *[10] - array of 10 pointers to int
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);
    NTypePtr ptrType = context.typeFactory->getPointerType(intType);
    std::vector<int> dims = {10};
    NTypePtr arrayOfPtrs = context.typeFactory->getArrayType(ptrType, dims);

    ASSERT_NE(arrayOfPtrs, nullptr);
    EXPECT_TRUE(arrayOfPtrs->isArray());

    NTypePtr elemType = arrayOfPtrs->getElementType(context);
    EXPECT_TRUE(elemType->isPointer());
}

TEST_F(TypeSystemTest, ComplexNestedType) {
    // int **[5][10] - 5x10 array of pointer to pointer to int
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);
    NTypePtr ptrPtrInt = context.typeFactory->getPointerType(intType, 2);
    std::vector<int> dims = {5, 10};
    NTypePtr complexType = context.typeFactory->getArrayType(ptrPtrInt, dims);

    ASSERT_NE(complexType, nullptr);
    EXPECT_TRUE(complexType->isArray());

    // 檢查層級
    NTypePtr level1 = complexType->getElementType(context);
    EXPECT_TRUE(level1->isArray());

    NTypePtr level2 = level1->getElementType(context);
    EXPECT_TRUE(level2->isPointer());
    EXPECT_EQ(level2->getName(), "int**");
}

// ==================== Type Helper Function Tests ====================

TEST_F(TypeSystemTest, VarTypeToString) {
    EXPECT_EQ(varTypeToString(VAR_TYPE_VOID), "void");
    EXPECT_EQ(varTypeToString(VAR_TYPE_INT), "int");
    EXPECT_EQ(varTypeToString(VAR_TYPE_FLOAT), "float");
    EXPECT_EQ(varTypeToString(VAR_TYPE_DOUBLE), "double");
    EXPECT_EQ(varTypeToString(VAR_TYPE_CHAR), "char");
}

TEST_F(TypeSystemTest, IsFloatingPointType) {
    EXPECT_TRUE(isFloatingPointType(VAR_TYPE_FLOAT));
    EXPECT_TRUE(isFloatingPointType(VAR_TYPE_DOUBLE));
    EXPECT_FALSE(isFloatingPointType(VAR_TYPE_INT));
    EXPECT_FALSE(isFloatingPointType(VAR_TYPE_CHAR));
}

TEST_F(TypeSystemTest, IsIntegerType) {
    EXPECT_TRUE(isIntegerType(VAR_TYPE_CHAR));
    EXPECT_TRUE(isIntegerType(VAR_TYPE_SHORT));
    EXPECT_TRUE(isIntegerType(VAR_TYPE_INT));
    EXPECT_TRUE(isIntegerType(VAR_TYPE_LONG));
    EXPECT_FALSE(isIntegerType(VAR_TYPE_FLOAT));
    EXPECT_FALSE(isIntegerType(VAR_TYPE_VOID));
}

// ==================== Error Handling Tests ====================

TEST_F(TypeSystemTest, NullTypeDescriptor) {
    NTypePtr type = context.typeFactory->realize(nullptr);
    EXPECT_EQ(type, nullptr) << "Null descriptor should return nullptr";
}

TEST_F(TypeSystemTest, PointerToVoid) {
    NTypePtr voidType = context.typeFactory->getBasicType(VAR_TYPE_VOID);
    NTypePtr voidPtr = context.typeFactory->getPointerType(voidType);

    ASSERT_NE(voidPtr, nullptr);
    EXPECT_EQ(voidPtr->getName(), "void*");
}

TEST_F(TypeSystemTest, ArrayOfVoid) {
    // 雖然 C 標準不允許 void 陣列，但我們的實作應該能處理
    NTypePtr voidType = context.typeFactory->getBasicType(VAR_TYPE_VOID);
    std::vector<int> dims = {10};
    NTypePtr voidArray = context.typeFactory->getArrayType(voidType, dims);

    // 可能返回 nullptr 或創建類型（取決於實作策略）
    // 這裡我們只檢查不會崩潰
    if (voidArray != nullptr) {
        EXPECT_TRUE(voidArray->isArray());
    }
}

// ==================== Memory Management Tests ====================

TEST_F(TypeSystemTest, TypeLifetime) {
    NTypePtr type1;
    {
        NTypePtr type2 = context.typeFactory->getBasicType(VAR_TYPE_INT);
        type1 = type2;
        // type2 離開作用域
    }
    // type1 應該仍然有效（因為 TypeFactory 持有）
    EXPECT_NE(type1, nullptr);
    EXPECT_EQ(type1->getVarType(), VAR_TYPE_INT);
}

TEST_F(TypeSystemTest, TypeFactoryOwnership) {
    // TypeFactory 應該持有所有創建的型別
    NTypePtr intType = context.typeFactory->getBasicType(VAR_TYPE_INT);
    void* originalPtr = intType.get();

    // 清空我們的引用
    intType.reset();

    // 再次獲取應該是同一個實例
    NTypePtr intType2 = context.typeFactory->getBasicType(VAR_TYPE_INT);
    EXPECT_EQ(intType2.get(), originalPtr) << "TypeFactory should maintain type lifetime";
}