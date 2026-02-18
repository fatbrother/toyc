#include <gtest/gtest.h>

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>

#include <memory>

#include "ast/define.hpp"
#include "ast/expression.hpp"
#include "ast/type.hpp"

using namespace toyc::ast;

class TypeManagerTest : public ::testing::Test {
protected:
    llvm::LLVMContext ctx;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<TypeManager> tm;

    void SetUp() override {
        module = std::make_unique<llvm::Module>("test", ctx);
        tm = std::make_unique<TypeManager>(ctx, *module);
    }
};

// ==================== PrimitiveType ====================

TEST_F(TypeManagerTest, PrimitiveTypeDeduplication) {
    TypeIdx intA = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx intB = tm->getPrimitiveIdx(VAR_TYPE_INT);
    EXPECT_EQ(intA, intB);

    // All eight primitives must have distinct indices
    TypeIdx voidIdx = tm->getPrimitiveIdx(VAR_TYPE_VOID);
    TypeIdx boolIdx = tm->getPrimitiveIdx(VAR_TYPE_BOOL);
    TypeIdx charIdx = tm->getPrimitiveIdx(VAR_TYPE_CHAR);
    TypeIdx shortIdx = tm->getPrimitiveIdx(VAR_TYPE_SHORT);
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx longIdx = tm->getPrimitiveIdx(VAR_TYPE_LONG);
    TypeIdx floatIdx = tm->getPrimitiveIdx(VAR_TYPE_FLOAT);
    TypeIdx doubleIdx = tm->getPrimitiveIdx(VAR_TYPE_DOUBLE);

    std::vector<TypeIdx> all = {voidIdx, boolIdx, charIdx, shortIdx, intIdx, longIdx, floatIdx, doubleIdx};
    std::set<TypeIdx> unique(all.begin(), all.end());
    EXPECT_EQ(unique.size(), all.size());
}

TEST_F(TypeManagerTest, PrimitiveTypeRealizeLLVMTypes) {
    EXPECT_EQ(tm->realize(tm->getPrimitiveIdx(VAR_TYPE_VOID)), llvm::Type::getVoidTy(ctx));
    EXPECT_EQ(tm->realize(tm->getPrimitiveIdx(VAR_TYPE_BOOL)), llvm::Type::getInt1Ty(ctx));
    EXPECT_EQ(tm->realize(tm->getPrimitiveIdx(VAR_TYPE_CHAR)), llvm::Type::getInt8Ty(ctx));
    EXPECT_EQ(tm->realize(tm->getPrimitiveIdx(VAR_TYPE_SHORT)), llvm::Type::getInt16Ty(ctx));
    EXPECT_EQ(tm->realize(tm->getPrimitiveIdx(VAR_TYPE_INT)), llvm::Type::getInt32Ty(ctx));
    EXPECT_EQ(tm->realize(tm->getPrimitiveIdx(VAR_TYPE_LONG)), llvm::Type::getInt64Ty(ctx));
    EXPECT_EQ(tm->realize(tm->getPrimitiveIdx(VAR_TYPE_FLOAT)), llvm::Type::getFloatTy(ctx));
    EXPECT_EQ(tm->realize(tm->getPrimitiveIdx(VAR_TYPE_DOUBLE)), llvm::Type::getDoubleTy(ctx));
}

TEST_F(TypeManagerTest, PrimitiveTypeGetReturnsCodegen) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    const auto* tc = dynamic_cast<const PrimitiveTypeCodegen*>(tm->get(intIdx));
    ASSERT_NE(tc, nullptr);
    EXPECT_EQ(tc->getVarType(), VAR_TYPE_INT);

    TypeIdx floatIdx = tm->getPrimitiveIdx(VAR_TYPE_FLOAT);
    const auto* ftc = dynamic_cast<const PrimitiveTypeCodegen*>(tm->get(floatIdx));
    ASSERT_NE(ftc, nullptr);
    EXPECT_EQ(ftc->getVarType(), VAR_TYPE_FLOAT);
}

TEST_F(TypeManagerTest, PrimitiveTypeIsFloatingPoint) {
    EXPECT_TRUE(tm->isFloatingPointType(tm->getPrimitiveIdx(VAR_TYPE_FLOAT)));
    EXPECT_TRUE(tm->isFloatingPointType(tm->getPrimitiveIdx(VAR_TYPE_DOUBLE)));

    EXPECT_FALSE(tm->isFloatingPointType(tm->getPrimitiveIdx(VAR_TYPE_INT)));
    EXPECT_FALSE(tm->isFloatingPointType(tm->getPrimitiveIdx(VAR_TYPE_CHAR)));
    EXPECT_FALSE(tm->isFloatingPointType(tm->getPrimitiveIdx(VAR_TYPE_BOOL)));
    EXPECT_FALSE(tm->isFloatingPointType(tm->getPrimitiveIdx(VAR_TYPE_VOID)));
    EXPECT_FALSE(tm->isFloatingPointType(InvalidTypeIdx));
}

TEST_F(TypeManagerTest, PrimitiveTypeGetTypeName) {
    EXPECT_EQ(tm->getTypeName(tm->realize(tm->getPrimitiveIdx(VAR_TYPE_BOOL))), "bool");
    EXPECT_EQ(tm->getTypeName(tm->realize(tm->getPrimitiveIdx(VAR_TYPE_CHAR))), "char");
    EXPECT_EQ(tm->getTypeName(tm->realize(tm->getPrimitiveIdx(VAR_TYPE_SHORT))), "short");
    EXPECT_EQ(tm->getTypeName(tm->realize(tm->getPrimitiveIdx(VAR_TYPE_INT))), "int");
    EXPECT_EQ(tm->getTypeName(tm->realize(tm->getPrimitiveIdx(VAR_TYPE_LONG))), "long");
    EXPECT_EQ(tm->getTypeName(tm->realize(tm->getPrimitiveIdx(VAR_TYPE_FLOAT))), "float");
    EXPECT_EQ(tm->getTypeName(tm->realize(tm->getPrimitiveIdx(VAR_TYPE_DOUBLE))), "double");
    EXPECT_EQ(tm->getTypeName(tm->realize(tm->getPrimitiveIdx(VAR_TYPE_VOID))), "void");
}

// ==================== PointerType ====================

TEST_F(TypeManagerTest, PointerTypeDeduplication) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx charIdx = tm->getPrimitiveIdx(VAR_TYPE_CHAR);

    TypeIdx ptrA = tm->getPointerIdx(intIdx, 1);
    TypeIdx ptrB = tm->getPointerIdx(intIdx, 1);
    EXPECT_EQ(ptrA, ptrB);

    // Different pointee → different TypeIdx
    TypeIdx charPtr = tm->getPointerIdx(charIdx, 1);
    EXPECT_NE(ptrA, charPtr);

    // Different level → different TypeIdx
    TypeIdx intPtrPtr = tm->getPointerIdx(intIdx, 2);
    EXPECT_NE(ptrA, intPtrPtr);
}

TEST_F(TypeManagerTest, PointerTypeRealizeLLVMType) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx ptrIdx = tm->getPointerIdx(intIdx, 1);
    llvm::Type* ptrType = tm->realize(ptrIdx);
    ASSERT_NE(ptrType, nullptr);
    EXPECT_TRUE(ptrType->isPointerTy());
}

TEST_F(TypeManagerTest, PointerTypePointeeAndLevel) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx ptrIdx = tm->getPointerIdx(intIdx, 1);
    const auto* tc = dynamic_cast<const PointerTypeCodegen*>(tm->get(ptrIdx));
    ASSERT_NE(tc, nullptr);
    EXPECT_EQ(tc->getPointeeIdx(), intIdx);
    EXPECT_EQ(tc->getLevel(), 1);
}

TEST_F(TypeManagerTest, PointerTypeMultiLevel) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx ptrPtr = tm->getPointerIdx(intIdx, 2);
    llvm::Type* type = tm->realize(ptrPtr);
    ASSERT_NE(type, nullptr);
    EXPECT_TRUE(type->isPointerTy());

    // Level-1 pointer must also be deduplication-cached as its own entry
    TypeIdx singlePtr = tm->getPointerIdx(intIdx, 1);
    EXPECT_NE(singlePtr, ptrPtr);
}

// ==================== ArrayType1D ====================

TEST_F(TypeManagerTest, ArrayType1DDeduplication) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);

    TypeIdx arr5a = tm->getArrayIdx(intIdx, {5});
    TypeIdx arr5b = tm->getArrayIdx(intIdx, {5});
    EXPECT_EQ(arr5a, arr5b);

    TypeIdx arr6 = tm->getArrayIdx(intIdx, {6});
    EXPECT_NE(arr5a, arr6);
}

TEST_F(TypeManagerTest, ArrayType1DRealizeLLVMType) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx arr = tm->getArrayIdx(intIdx, {10});
    llvm::Type* type = tm->realize(arr);
    ASSERT_NE(type, nullptr);
    ASSERT_TRUE(type->isArrayTy());
    auto* arrType = llvm::cast<llvm::ArrayType>(type);
    EXPECT_EQ(arrType->getNumElements(), 10u);
    EXPECT_EQ(arrType->getElementType(), llvm::Type::getInt32Ty(ctx));
}

TEST_F(TypeManagerTest, ArrayType1DElementIdx) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx arr = tm->getArrayIdx(intIdx, {7});
    const auto* tc = dynamic_cast<const ArrayTypeCodegen*>(tm->get(arr));
    ASSERT_NE(tc, nullptr);
    EXPECT_EQ(tc->getElementIdx(), intIdx);
}

TEST_F(TypeManagerTest, ArrayType1DGetSize) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx arr = tm->getArrayIdx(intIdx, {42});
    const auto* tc = dynamic_cast<const ArrayTypeCodegen*>(tm->get(arr));
    ASSERT_NE(tc, nullptr);
    EXPECT_EQ(tc->getSize(), 42);
}

// ==================== ArrayTypeND ====================

TEST_F(TypeManagerTest, ArrayTypeNDChainedElementIdx) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);

    // int[2][3]: outer array has 2 elements, each element is int[3]
    TypeIdx arr2x3 = tm->getArrayIdx(intIdx, {2, 3});
    TypeIdx arr3 = tm->getArrayIdx(intIdx, {3});

    const auto* outerTc = dynamic_cast<const ArrayTypeCodegen*>(tm->get(arr2x3));
    ASSERT_NE(outerTc, nullptr);
    // Element of int[2][3] must be int[3], not int
    EXPECT_EQ(outerTc->getElementIdx(), arr3);
}

TEST_F(TypeManagerTest, ArrayTypeNDRealizeLLVMType) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx arr2x3 = tm->getArrayIdx(intIdx, {2, 3});

    llvm::Type* outer = tm->realize(arr2x3);
    ASSERT_NE(outer, nullptr);
    ASSERT_TRUE(outer->isArrayTy());

    auto* outerArr = llvm::cast<llvm::ArrayType>(outer);
    EXPECT_EQ(outerArr->getNumElements(), 2u);

    llvm::Type* inner = outerArr->getElementType();
    ASSERT_TRUE(inner->isArrayTy());
    auto* innerArr = llvm::cast<llvm::ArrayType>(inner);
    EXPECT_EQ(innerArr->getNumElements(), 3u);
    EXPECT_EQ(innerArr->getElementType(), llvm::Type::getInt32Ty(ctx));
}

TEST_F(TypeManagerTest, ArrayTypeNDDeduplication) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);

    TypeIdx a = tm->getArrayIdx(intIdx, {2, 3});
    TypeIdx b = tm->getArrayIdx(intIdx, {2, 3});
    EXPECT_EQ(a, b);

    // Different outer dimension → different TypeIdx
    TypeIdx c = tm->getArrayIdx(intIdx, {4, 3});
    EXPECT_NE(a, c);
}

// ==================== StructType ====================

TEST_F(TypeManagerTest, StructTypeForwardDeclaration) {
    TypeIdx idx = tm->getStructIdx("Opaque", nullptr);
    EXPECT_NE(idx, InvalidTypeIdx);

    llvm::Type* type = tm->realize(idx);
    ASSERT_NE(type, nullptr);
    auto* st = llvm::dyn_cast<llvm::StructType>(type);
    ASSERT_NE(st, nullptr);
    // Forward-declared struct should be opaque (no body)
    EXPECT_TRUE(st->isOpaque());
}

TEST_F(TypeManagerTest, StructTypeMemberLookup) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx floatIdx = tm->getPrimitiveIdx(VAR_TYPE_FLOAT);

    // Build member list: { int x; float y; }
    auto* declY = new NDeclarator("y");
    auto* memberY = new NStructDeclaration(floatIdx, declY);

    auto* declX = new NDeclarator("x");
    auto* memberX = new NStructDeclaration(intIdx, declX);
    memberX->next = memberY;

    TypeIdx structIdx = tm->getStructIdx("Point", memberX);
    // StructTypeCodegen takes ownership of memberX — do NOT free it

    const auto* tc = dynamic_cast<const StructTypeCodegen*>(tm->get(structIdx));
    ASSERT_NE(tc, nullptr);

    EXPECT_EQ(tc->getMemberIndex("x"), 0);
    EXPECT_EQ(tc->getMemberIndex("y"), 1);
    EXPECT_EQ(tc->getMemberTypeIdx(0), intIdx);
    EXPECT_EQ(tc->getMemberTypeIdx(1), floatIdx);
}

TEST_F(TypeManagerTest, StructTypeForwardThenDefine) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);

    // Forward declaration
    TypeIdx fwdIdx = tm->getStructIdx("Node", nullptr);
    EXPECT_NE(fwdIdx, InvalidTypeIdx);

    // Definition — same name, now with a member
    auto* decl = new NDeclarator("val");
    auto* member = new NStructDeclaration(intIdx, decl);
    TypeIdx defIdx = tm->getStructIdx("Node", member);

    // Must return the same TypeIdx
    EXPECT_EQ(fwdIdx, defIdx);

    // Member info must be populated
    const auto* tc = dynamic_cast<const StructTypeCodegen*>(tm->get(defIdx));
    ASSERT_NE(tc, nullptr);
    EXPECT_TRUE(tc->hasMembers());
    EXPECT_EQ(tc->getMemberIndex("val"), 0);
    EXPECT_EQ(tc->getMemberTypeIdx(0), intIdx);
}

TEST_F(TypeManagerTest, StructTypeUnknownMember) {
    TypeIdx idx = tm->getStructIdx("Empty", nullptr);
    const auto* tc = dynamic_cast<const StructTypeCodegen*>(tm->get(idx));
    ASSERT_NE(tc, nullptr);
    EXPECT_EQ(tc->getMemberIndex("nonexistent"), -1);
    EXPECT_EQ(tc->getMemberTypeIdx(-1), InvalidTypeIdx);
    EXPECT_EQ(tc->getMemberTypeIdx(999), InvalidTypeIdx);
}

// ==================== CommonType ====================

TEST_F(TypeManagerTest, CommonTypeSameType) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    EXPECT_EQ(tm->getCommonTypeIdx(intIdx, intIdx), intIdx);

    TypeIdx doubleIdx = tm->getPrimitiveIdx(VAR_TYPE_DOUBLE);
    EXPECT_EQ(tm->getCommonTypeIdx(doubleIdx, doubleIdx), doubleIdx);
}

TEST_F(TypeManagerTest, CommonTypeIntegerRank) {
    TypeIdx charIdx = tm->getPrimitiveIdx(VAR_TYPE_CHAR);
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx longIdx = tm->getPrimitiveIdx(VAR_TYPE_LONG);

    EXPECT_EQ(tm->getCommonTypeIdx(charIdx, intIdx), intIdx);
    EXPECT_EQ(tm->getCommonTypeIdx(intIdx, longIdx), longIdx);
    EXPECT_EQ(tm->getCommonTypeIdx(charIdx, longIdx), longIdx);
}

TEST_F(TypeManagerTest, CommonTypeFloatBeatsInt) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx longIdx = tm->getPrimitiveIdx(VAR_TYPE_LONG);
    TypeIdx floatIdx = tm->getPrimitiveIdx(VAR_TYPE_FLOAT);

    EXPECT_EQ(tm->getCommonTypeIdx(intIdx, floatIdx), floatIdx);
    EXPECT_EQ(tm->getCommonTypeIdx(longIdx, floatIdx), floatIdx);
}

TEST_F(TypeManagerTest, CommonTypeDoubleBeatsFloat) {
    TypeIdx floatIdx = tm->getPrimitiveIdx(VAR_TYPE_FLOAT);
    TypeIdx doubleIdx = tm->getPrimitiveIdx(VAR_TYPE_DOUBLE);

    EXPECT_EQ(tm->getCommonTypeIdx(floatIdx, doubleIdx), doubleIdx);
    EXPECT_EQ(tm->getCommonTypeIdx(doubleIdx, floatIdx), doubleIdx);
}

TEST_F(TypeManagerTest, CommonTypeBoolIsLowest) {
    TypeIdx boolIdx = tm->getPrimitiveIdx(VAR_TYPE_BOOL);
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx floatIdx = tm->getPrimitiveIdx(VAR_TYPE_FLOAT);

    EXPECT_EQ(tm->getCommonTypeIdx(boolIdx, intIdx), intIdx);
    EXPECT_EQ(tm->getCommonTypeIdx(boolIdx, floatIdx), floatIdx);
}

// ==================== InvalidIdx ====================

TEST_F(TypeManagerTest, InvalidIdxGetReturnsNull) {
    EXPECT_EQ(tm->get(InvalidTypeIdx), nullptr);
}

TEST_F(TypeManagerTest, InvalidIdxRealizeReturnsNull) {
    EXPECT_EQ(tm->realize(InvalidTypeIdx), nullptr);
}

TEST_F(TypeManagerTest, InvalidIdxIsFloatingPointFalse) {
    EXPECT_FALSE(tm->isFloatingPointType(InvalidTypeIdx));
}

// ==================== QualifiedPointerType ====================

TEST_F(TypeManagerTest, QualifiedPointerConstDedup) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx rawPtr = tm->getPointerIdx(intIdx, 1);
    TypeIdx a = tm->getQualifiedIdx(rawPtr, QUAL_CONST);
    TypeIdx b = tm->getQualifiedIdx(rawPtr, QUAL_CONST);
    EXPECT_EQ(a, b);
}

TEST_F(TypeManagerTest, QualifiedPointerVsUnqualified) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx rawPtr = tm->getPointerIdx(intIdx, 1);
    TypeIdx unqualified = rawPtr;
    TypeIdx constPtr = tm->getQualifiedIdx(rawPtr, QUAL_CONST);
    TypeIdx volatilePtr = tm->getQualifiedIdx(rawPtr, QUAL_VOLATILE);
    EXPECT_NE(unqualified, constPtr);
    EXPECT_NE(unqualified, volatilePtr);
    EXPECT_NE(constPtr, volatilePtr);
}

TEST_F(TypeManagerTest, QualifiedPointerConstFlag) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx rawPtr = tm->getPointerIdx(intIdx, 1);
    TypeIdx constPtr = tm->getQualifiedIdx(rawPtr, QUAL_CONST);
    TypeIdx plain = rawPtr;
    EXPECT_TRUE(tm->isConstQualified(constPtr));
    EXPECT_FALSE(tm->isConstQualified(plain));
    EXPECT_FALSE(tm->isVolatileQualified(constPtr));
}

TEST_F(TypeManagerTest, QualifiedPointerVolatileFlag) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx rawPtr = tm->getPointerIdx(intIdx, 1);
    TypeIdx volatilePtr = tm->getQualifiedIdx(rawPtr, QUAL_VOLATILE);
    TypeIdx plain = rawPtr;
    EXPECT_TRUE(tm->isVolatileQualified(volatilePtr));
    EXPECT_FALSE(tm->isVolatileQualified(plain));
    EXPECT_FALSE(tm->isConstQualified(volatilePtr));
}

TEST_F(TypeManagerTest, QualifiedPointerBothQualifiers) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx rawPtr = tm->getPointerIdx(intIdx, 1);
    TypeIdx bothPtr = tm->getQualifiedIdx(rawPtr, QUAL_CONST | QUAL_VOLATILE);
    EXPECT_TRUE(tm->isConstQualified(bothPtr));
    EXPECT_TRUE(tm->isVolatileQualified(bothPtr));
}

TEST_F(TypeManagerTest, QualifiedPointerOnPrimitiveReturnsFalse) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    EXPECT_FALSE(tm->isConstQualified(intIdx));
    EXPECT_FALSE(tm->isVolatileQualified(intIdx));
}

// ==================== QualifiedBaseType ====================

TEST_F(TypeManagerTest, QualifiedBaseTypeConstInt) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx constIntIdx = tm->getQualifiedIdx(intIdx, QUAL_CONST);
    EXPECT_NE(constIntIdx, intIdx);
    // LLVM type is still i32
    EXPECT_EQ(tm->realize(constIntIdx), llvm::Type::getInt32Ty(ctx));
}

TEST_F(TypeManagerTest, QualifiedBaseTypeDedup) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx a = tm->getQualifiedIdx(intIdx, QUAL_CONST);
    TypeIdx b = tm->getQualifiedIdx(intIdx, QUAL_CONST);
    EXPECT_EQ(a, b);
}

TEST_F(TypeManagerTest, QualifiedBaseTypeIsConstTrue) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx constIntIdx = tm->getQualifiedIdx(intIdx, QUAL_CONST);
    EXPECT_TRUE(tm->isConstQualified(constIntIdx));
}

TEST_F(TypeManagerTest, QualifiedBaseTypeIsConstFalse) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    EXPECT_FALSE(tm->isConstQualified(intIdx));
}

TEST_F(TypeManagerTest, QualifiedBaseTypePointerToConstInt) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx constIntIdx = tm->getQualifiedIdx(intIdx, QUAL_CONST);
    TypeIdx ptrToConstInt = tm->getPointerIdx(constIntIdx, 1);
    EXPECT_TRUE(tm->realize(ptrToConstInt)->isPointerTy());
}

TEST_F(TypeManagerTest, QualifiedBaseTypeUnqualify) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    TypeIdx constIntIdx = tm->getQualifiedIdx(intIdx, QUAL_CONST);
    EXPECT_EQ(tm->unqualify(constIntIdx), intIdx);
}

TEST_F(TypeManagerTest, QualifiedBaseTypeUnqualifyNonQualified) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    EXPECT_EQ(tm->unqualify(intIdx), intIdx);
}

TEST_F(TypeManagerTest, QualifiedBaseTypeNoneReturnsBase) {
    TypeIdx intIdx = tm->getPrimitiveIdx(VAR_TYPE_INT);
    // getQualifiedIdx with QUAL_NONE returns the base unchanged
    EXPECT_EQ(tm->getQualifiedIdx(intIdx, QUAL_NONE), intIdx);
}
