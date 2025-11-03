#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <string>
#include "utility/parse_file.hpp"
#include "utility/error_handler.hpp"

// 參數化測試的結構
struct SyntaxTestCase {
    std::string testName;
    std::string inputFile;
    std::string description;
};

// 自訂測試名稱生成函數
struct SyntaxTestNameGenerator {
    std::string operator()(const ::testing::TestParamInfo<SyntaxTestCase>& info) const {
        return info.param.testName;
    }
};

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

// ============================================================================
// 參數化測試：語法解析測試
// ============================================================================

class SyntaxParsingTest : public SyntaxTest,
                          public ::testing::WithParamInterface<SyntaxTestCase> {
};

TEST_P(SyntaxParsingTest, FileParsing) {
    const SyntaxTestCase& testCase = GetParam();

    ASSERT_TRUE(fileExists(testCase.inputFile))
        << "測試檔案不存在: " << testCase.inputFile;

    EXPECT_TRUE(testFileParsing(testCase.inputFile))
        << testCase.description;
}

INSTANTIATE_TEST_SUITE_P(
    AllSyntaxTests,
    SyntaxParsingTest,
    ::testing::Values(
        // 資料類型測試
        SyntaxTestCase{"basic_data_types", "tests/fixtures/syntax/data_types/basic_types.c", "基本資料類型解析失敗"},
        SyntaxTestCase{"pointer_types", "tests/fixtures/syntax/data_types/pointer_types.c", "指標類型解析失敗"},
        SyntaxTestCase{"void_type", "tests/fixtures/syntax/data_types/void_type.c", "void 類型解析失敗"},

        // 運算符測試
        SyntaxTestCase{"arithmetic_operators", "tests/fixtures/syntax/operators/arithmetic.c", "算術運算符解析失敗"},
        SyntaxTestCase{"comparison_operators", "tests/fixtures/syntax/operators/comparison.c", "比較運算符解析失敗"},
        SyntaxTestCase{"logical_operators", "tests/fixtures/syntax/operators/logical.c", "邏輯運算符解析失敗"},
        SyntaxTestCase{"bitwise_operators", "tests/fixtures/syntax/operators/bitwise.c", "位元運算符解析失敗"},
        SyntaxTestCase{"assignment_operators", "tests/fixtures/syntax/operators/assignment.c", "賦值運算符解析失敗"},
        SyntaxTestCase{"increment_decrement", "tests/fixtures/syntax/operators/increment_decrement.c", "遞增遞減運算符解析失敗"},
        SyntaxTestCase{"address_dereference", "tests/fixtures/syntax/operators/address_dereference.c", "位址和解參考運算符解析失敗"},
        SyntaxTestCase{"ternary_operator", "tests/fixtures/syntax/operators/ternary.c", "三元運算符解析失敗"},
        SyntaxTestCase{"short_circuit", "tests/fixtures/syntax/operators/short_circuit.c", "短路求值解析失敗"},

        // 控制流測試
        SyntaxTestCase{"if_else", "tests/fixtures/syntax/control_flow/if_else.c", "if-else 語句解析失敗"},
        SyntaxTestCase{"for_loop", "tests/fixtures/syntax/control_flow/for_loop.c", "for 迴圈解析失敗"},
        SyntaxTestCase{"while_loop", "tests/fixtures/syntax/control_flow/while_loop.c", "while 迴圈解析失敗"},
        SyntaxTestCase{"do_while_loop", "tests/fixtures/syntax/control_flow/do_while_loop.c", "do-while 迴圈解析失敗"},
        SyntaxTestCase{"return_statement", "tests/fixtures/syntax/control_flow/return_statement.c", "return 語句解析失敗"},
        SyntaxTestCase{"break_continue", "tests/fixtures/syntax/control_flow/break_continue.c", "break/continue 語句解析失敗"},
        SyntaxTestCase{"goto_statement", "tests/fixtures/syntax/control_flow/goto_statement.c", "goto 語句解析失敗"},
        SyntaxTestCase{"switch_statement", "tests/fixtures/syntax/control_flow/switch_statement.c", "switch 語句解析失敗"},

        // 函數測試
        SyntaxTestCase{"function_definition", "tests/fixtures/syntax/functions/function_definition.c", "函數定義解析失敗"},
        SyntaxTestCase{"function_parameters", "tests/fixtures/syntax/functions/function_parameters.c", "函數參數解析失敗"},
        SyntaxTestCase{"function_calls", "tests/fixtures/syntax/functions/function_calls.c", "函數調用解析失敗"},
        SyntaxTestCase{"variadic_functions", "tests/fixtures/syntax/functions/variadic_functions.c", "可變參數函數解析失敗"},

        // 變數測試
        SyntaxTestCase{"variable_declaration", "tests/fixtures/syntax/variables/variable_declaration.c", "變數聲明解析失敗"},
        SyntaxTestCase{"variable_initialization", "tests/fixtures/syntax/variables/variable_initialization.c", "變數初始化解析失敗"},
        SyntaxTestCase{"variable_scope", "tests/fixtures/syntax/variables/variable_scope.c", "變數作用域解析失敗"},

        // 字面值測試
        SyntaxTestCase{"integer_literals", "tests/fixtures/syntax/literals/integer_literals.c", "整數字面值解析失敗"},
        SyntaxTestCase{"float_literals", "tests/fixtures/syntax/literals/float_literals.c", "浮點字面值解析失敗"},
        SyntaxTestCase{"string_literals", "tests/fixtures/syntax/literals/string_literals.c", "字串字面值解析失敗"},

        // 複合測試
        SyntaxTestCase{"complex_expressions", "tests/fixtures/syntax/complex_expressions.c", "複雜表達式解析失敗"},
        SyntaxTestCase{"complete_program", "tests/fixtures/syntax/complete_program.c", "完整程式解析失敗"},

        // Struct 語法測試
        SyntaxTestCase{"basic_struct", "tests/fixtures/syntax/structures/basic_struct.c", "基本 struct 定義解析失敗"},
        SyntaxTestCase{"simple_struct_definition", "tests/fixtures/syntax/structures/simple_struct_definition.c", "簡單 struct 定義解析失敗"},
        SyntaxTestCase{"struct_initialization", "tests/fixtures/syntax/structures/struct_initialization.c", "struct 初始化解析失敗"},
        SyntaxTestCase{"anonymous_struct", "tests/fixtures/syntax/structures/anonymous_struct.c", "匿名 struct 解析失敗"},
        SyntaxTestCase{"forward_declaration", "tests/fixtures/syntax/structures/forward_declaration.c", "struct 前向聲明解析失敗"},
        SyntaxTestCase{"struct_forward_only", "tests/fixtures/syntax/structures/struct_forward_only.c", "僅前向聲明的 struct 解析失敗"},
        SyntaxTestCase{"nested_struct", "tests/fixtures/syntax/structures/nested_struct.c", "嵌套 struct 解析失敗"},
        SyntaxTestCase{"struct_pointer", "tests/fixtures/syntax/structures/struct_pointer.c", "struct 指標解析失敗"},
        SyntaxTestCase{"struct_as_parameter", "tests/fixtures/syntax/structures/struct_as_parameter.c", "struct 作為參數解析失敗"},
        SyntaxTestCase{"complex_struct", "tests/fixtures/syntax/structures/complex_struct.c", "複雜 struct 解析失敗"},
        SyntaxTestCase{"struct_variable_declaration", "tests/fixtures/syntax/structures/struct_variable_declaration.c", "struct 變數聲明解析失敗"},

        // Array 語法測試
        SyntaxTestCase{"array_declaration", "tests/fixtures/syntax/arrays/array_declaration.c", "陣列聲明解析失敗"},
        SyntaxTestCase{"array_initialization", "tests/fixtures/syntax/arrays/array_initialization.c", "陣列初始化解析失敗"},
        SyntaxTestCase{"array_indexing", "tests/fixtures/syntax/arrays/array_indexing.c", "陣列索引解析失敗"},
        SyntaxTestCase{"multidimensional_arrays", "tests/fixtures/syntax/arrays/multidimensional_arrays.c", "多維陣列解析失敗"}
    ),
    SyntaxTestNameGenerator()
);