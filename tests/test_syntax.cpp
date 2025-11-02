#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <string>
#include "utility/parse_file.hpp"
#include "utility/error_handler.hpp"

class SyntaxTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 測試前的設置
    }

    void TearDown() override {
        // 測試後的清理
    }

    // 輔助方法：測試檔案是否能夠成功解析
    bool testFileParsing(const std::string& filepath) {
        int result = toyc::parser::parseFileWithPreprocessor(filepath);
        if (result != 0) {
            std::cerr << "Parsing failed for file: " << filepath << std::endl;
            return false;
        }

        return true;
    }

    // 輔助方法：檢查檔案是否存在且可讀
    bool fileExists(const std::string& filepath) {
        std::ifstream file(filepath);
        return file.good();
    }
};

// 資料類型測試
TEST_F(SyntaxTest, BasicDataTypes) {
    std::string filepath = "tests/fixtures/syntax/data_types/basic_types.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "基本資料類型解析失敗";
}

TEST_F(SyntaxTest, PointerTypes) {
    std::string filepath = "tests/fixtures/syntax/data_types/pointer_types.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "指標類型解析失敗";
}

TEST_F(SyntaxTest, VoidType) {
    std::string filepath = "tests/fixtures/syntax/data_types/void_type.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "void 類型解析失敗";
}

// 運算符測試
TEST_F(SyntaxTest, ArithmeticOperators) {
    std::string filepath = "tests/fixtures/syntax/operators/arithmetic.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "算術運算符解析失敗";
}

TEST_F(SyntaxTest, ComparisonOperators) {
    std::string filepath = "tests/fixtures/syntax/operators/comparison.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "比較運算符解析失敗";
}

TEST_F(SyntaxTest, LogicalOperators) {
    std::string filepath = "tests/fixtures/syntax/operators/logical.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "邏輯運算符解析失敗";
}

TEST_F(SyntaxTest, BitwiseOperators) {
    std::string filepath = "tests/fixtures/syntax/operators/bitwise.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "位元運算符解析失敗";
}

TEST_F(SyntaxTest, AssignmentOperators) {
    std::string filepath = "tests/fixtures/syntax/operators/assignment.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "賦值運算符解析失敗";
}

TEST_F(SyntaxTest, IncrementDecrementOperators) {
    std::string filepath = "tests/fixtures/syntax/operators/increment_decrement.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "遞增遞減運算符解析失敗";
}

TEST_F(SyntaxTest, AddressDereferenceOperators) {
    std::string filepath = "tests/fixtures/syntax/operators/address_dereference.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "位址和解參考運算符解析失敗";
}

TEST_F(SyntaxTest, TernaryOperator) {
    std::string filepath = "tests/fixtures/syntax/operators/ternary.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "三元運算符解析失敗";
}

// 控制流測試
TEST_F(SyntaxTest, IfElseStatements) {
    std::string filepath = "tests/fixtures/syntax/control_flow/if_else.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "if-else 語句解析失敗";
}

TEST_F(SyntaxTest, ForLoops) {
    std::string filepath = "tests/fixtures/syntax/control_flow/for_loop.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "for 迴圈解析失敗";
}

TEST_F(SyntaxTest, WhileLoops) {
    std::string filepath = "tests/fixtures/syntax/control_flow/while_loop.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "while 迴圈解析失敗";
}

TEST_F(SyntaxTest, DoWhileLoops) {
    std::string filepath = "tests/fixtures/syntax/control_flow/do_while_loop.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "do-while 迴圈解析失敗";
}

TEST_F(SyntaxTest, ReturnStatements) {
    std::string filepath = "tests/fixtures/syntax/control_flow/return_statement.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "return 語句解析失敗";
}

TEST_F(SyntaxTest, BreakContinueStatements) {
    std::string filepath = "tests/fixtures/syntax/control_flow/break_continue.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "break/continue 語句解析失敗";
}

TEST_F(SyntaxTest, GotoStatements) {
    std::string filepath = "tests/fixtures/syntax/control_flow/goto_statement.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "goto 語句解析失敗";
}

TEST_F(SyntaxTest, SwitchStatements) {
    std::string filepath = "tests/fixtures/syntax/control_flow/switch_statement.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "switch 語句解析失敗";
}

// 函數測試
TEST_F(SyntaxTest, FunctionDefinitions) {
    std::string filepath = "tests/fixtures/syntax/functions/function_definition.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "函數定義解析失敗";
}

TEST_F(SyntaxTest, FunctionParameters) {
    std::string filepath = "tests/fixtures/syntax/functions/function_parameters.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "函數參數解析失敗";
}

TEST_F(SyntaxTest, FunctionCalls) {
    std::string filepath = "tests/fixtures/syntax/functions/function_calls.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "函數調用解析失敗";
}

TEST_F(SyntaxTest, VariadicFunctions) {
    std::string filepath = "tests/fixtures/syntax/functions/variadic_functions.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "可變參數函數解析失敗";
}

// 變數測試
TEST_F(SyntaxTest, VariableDeclarations) {
    std::string filepath = "tests/fixtures/syntax/variables/variable_declaration.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "變數聲明解析失敗";
}

TEST_F(SyntaxTest, VariableInitialization) {
    std::string filepath = "tests/fixtures/syntax/variables/variable_initialization.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "變數初始化解析失敗";
}

TEST_F(SyntaxTest, VariableScope) {
    std::string filepath = "tests/fixtures/syntax/variables/variable_scope.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "變數作用域解析失敗";
}

// 字面值測試
TEST_F(SyntaxTest, IntegerLiterals) {
    std::string filepath = "tests/fixtures/syntax/literals/integer_literals.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "整數字面值解析失敗";
}

TEST_F(SyntaxTest, FloatLiterals) {
    std::string filepath = "tests/fixtures/syntax/literals/float_literals.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "浮點字面值解析失敗";
}

TEST_F(SyntaxTest, StringLiterals) {
    std::string filepath = "tests/fixtures/syntax/literals/string_literals.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "字串字面值解析失敗";
}

// 複合測試
TEST_F(SyntaxTest, ComplexExpressions) {
    std::string filepath = "tests/fixtures/syntax/complex_expressions.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "複雜表達式解析失敗";
}

TEST_F(SyntaxTest, CompleteProgram) {
    std::string filepath = "tests/fixtures/syntax/complete_program.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "完整程式解析失敗";
}

// Struct 語法測試
TEST_F(SyntaxTest, BasicStruct) {
    std::string filepath = "tests/fixtures/syntax/structures/basic_struct.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "基本 struct 定義解析失敗";
}

TEST_F(SyntaxTest, SimpleStructDefinition) {
    std::string filepath = "tests/fixtures/syntax/structures/simple_struct_definition.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "簡單 struct 定義解析失敗";
}

TEST_F(SyntaxTest, StructInitialization) {
    std::string filepath = "tests/fixtures/syntax/structures/struct_initialization.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "struct 初始化解析失敗";
}

TEST_F(SyntaxTest, AnonymousStruct) {
    std::string filepath = "tests/fixtures/syntax/structures/anonymous_struct.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "匿名 struct 解析失敗";
}

TEST_F(SyntaxTest, ForwardDeclaration) {
    std::string filepath = "tests/fixtures/syntax/structures/forward_declaration.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "struct 前向聲明解析失敗";
}

TEST_F(SyntaxTest, StructForwardOnly) {
    std::string filepath = "tests/fixtures/syntax/structures/struct_forward_only.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "僅前向聲明的 struct 解析失敗";
}

TEST_F(SyntaxTest, NestedStruct) {
    std::string filepath = "tests/fixtures/syntax/structures/nested_struct.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "嵌套 struct 解析失敗";
}

TEST_F(SyntaxTest, StructPointer) {
    std::string filepath = "tests/fixtures/syntax/structures/struct_pointer.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "struct 指標解析失敗";
}

TEST_F(SyntaxTest, StructAsParameter) {
    std::string filepath = "tests/fixtures/syntax/structures/struct_as_parameter.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "struct 作為參數解析失敗";
}

TEST_F(SyntaxTest, ComplexStruct) {
    std::string filepath = "tests/fixtures/syntax/structures/complex_struct.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "複雜 struct 解析失敗";
}

TEST_F(SyntaxTest, StructVariableDeclaration) {
    std::string filepath = "tests/fixtures/syntax/structures/struct_variable_declaration.c";
    ASSERT_TRUE(fileExists(filepath)) << "測試檔案不存在: " << filepath;
    EXPECT_TRUE(testFileParsing(filepath)) << "struct 變數聲明解析失敗";
}