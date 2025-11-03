#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <sys/wait.h>
#include <unistd.h>

namespace fs = std::filesystem;

// 執行結果測試的參數化結構
struct ExecutionTestCase {
    std::string testName;
    std::string inputFile;
    int expectedExitCode;
    std::string description;
};

// 為 ExecutionTestCase 提供輸出運算子,避免顯示亂碼
std::ostream& operator<<(std::ostream& os, const ExecutionTestCase& testCase) {
    os << "ExecutionTestCase{" << testCase.testName << ", " << testCase.inputFile << ", exitCode=" << testCase.expectedExitCode << "}";
    return os;
}

// LLVM IR 生成測試的參數化結構
struct LLVMIRTestCase {
    std::string testName;
    std::string inputFile;
    std::vector<std::string> expectedPatterns;  // 期望在 IR 中找到的模式
    std::string description;
};

// 為 LLVMIRTestCase 提供輸出運算子,避免顯示亂碼
std::ostream& operator<<(std::ostream& os, const LLVMIRTestCase& testCase) {
    os << "LLVMIRTestCase{" << testCase.testName << ", " << testCase.inputFile << "}";
    return os;
}

// 自訂測試名稱生成函數
struct ExecutionTestNameGenerator {
    std::string operator()(const ::testing::TestParamInfo<ExecutionTestCase>& info) const {
        return info.param.testName;
    }
};

struct LLVMIRTestNameGenerator {
    std::string operator()(const ::testing::TestParamInfo<LLVMIRTestCase>& info) const {
        return info.param.testName;
    }
};

class OutputTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 確保測試目錄存在
        test_output_dir = "tests/output_temp";
        fs::create_directories(test_output_dir);
    }

    void TearDown() override {
        // 清理測試產生的檔案
        if (fs::exists(test_output_dir)) {
            fs::remove_all(test_output_dir);
        }
    }

    std::string test_output_dir;

    // 輔助方法：編譯 C 檔案並返回是否成功
    bool compileFile(const std::string& inputFile, const std::string& outputFile) {
        std::string command = "./toyc -o " + outputFile + " " + inputFile;
        int result = system(command.c_str());
        return WEXITSTATUS(result) == 0;
    }

    // 輔助方法：編譯 C 檔案生成 LLVM IR
    bool generateLLVMIR(const std::string& inputFile, const std::string& llvmFile) {
        std::string command = "./toyc " + inputFile + " -l -o " + llvmFile;
        int result = system(command.c_str());
        return WEXITSTATUS(result) == 0;
    }

    // 輔助方法：執行程式並返回退出碼
    int executeProgram(const std::string& executable) {
        int result = system(executable.c_str());
        return WEXITSTATUS(result);
    }

    // 輔助方法：檢查檔案是否存在
    bool fileExists(const std::string& filepath) {
        return fs::exists(filepath);
    }

    // 輔助方法：讀取檔案內容
    std::string readFile(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return "";
        }

        std::string content;
        std::string line;
        while (std::getline(file, line)) {
            content += line + "\n";
        }
        return content;
    }

    // 輔助方法：檢查 LLVM IR 是否包含特定內容
    bool llvmIRContains(const std::string& llvmFile, const std::string& pattern) {
        std::string content = readFile(llvmFile);
        return content.find(pattern) != std::string::npos;
    }

    // 輔助方法：驗證並編譯測試檔案
    bool validateAndCompile(const std::string& inputFile, const std::string& execFile) {
        if (!fileExists(inputFile)) {
            ADD_FAILURE() << "測試檔案不存在: " << inputFile;
            return false;
        }

        if (!compileFile(inputFile, execFile)) {
            ADD_FAILURE() << "編譯失敗: " << inputFile;
            return false;
        }

        return true;
    }
};

// ============================================================================
// 參數化測試：LLVM IR 生成測試
// ============================================================================

class LLVMIRGenerationTest : public OutputTest,
                             public ::testing::WithParamInterface<LLVMIRTestCase> {
};

TEST_P(LLVMIRGenerationTest, IRGeneration) {
    const LLVMIRTestCase& testCase = GetParam();
    std::string llvmFile = test_output_dir + "/" + testCase.testName + ".ll";

    ASSERT_TRUE(fileExists(testCase.inputFile))
        << "測試檔案不存在: " << testCase.inputFile;

    EXPECT_TRUE(generateLLVMIR(testCase.inputFile, llvmFile))
        << testCase.description;

    EXPECT_TRUE(fileExists(llvmFile))
        << "LLVM IR 檔案未生成";

    // 檢查所有期望的模式
    for (const auto& pattern : testCase.expectedPatterns) {
        EXPECT_TRUE(llvmIRContains(llvmFile, pattern))
            << "LLVM IR 缺少: " << pattern;
    }
}

INSTANTIATE_TEST_SUITE_P(
    AllLLVMIRTests,
    LLVMIRGenerationTest,
    ::testing::Values(
        // 基本功能測試
        LLVMIRTestCase{"return_constant", "tests/fixtures/output/simple_programs/return_constant.c",
                       {"define", "main", "ret"}, "基本 LLVM IR 生成失敗"},

        // 算術運算測試
        LLVMIRTestCase{"addition", "tests/fixtures/output/calculations/addition.c",
                       {"add"}, "加法 LLVM IR 生成失敗"},
        LLVMIRTestCase{"multiplication", "tests/fixtures/output/calculations/multiplication.c",
                       {"mul"}, "乘法 LLVM IR 生成失敗"},
        LLVMIRTestCase{"complex_arithmetic", "tests/fixtures/output/calculations/complex_arithmetic.c",
                       {"add", "mul", "sub"}, "複雜算術 LLVM IR 生成失敗"},

        // 函數測試
        LLVMIRTestCase{"simple_function", "tests/fixtures/output/functions/simple_function.c",
                       {"call", "define"}, "函數調用 LLVM IR 生成失敗"},
        LLVMIRTestCase{"recursive_function", "tests/fixtures/output/functions/recursive_function.c",
                       {"call", "icmp", "br"}, "遞歸函數 LLVM IR 生成失敗"},

        // 運算符測試
        LLVMIRTestCase{"shift_operations", "tests/fixtures/output/operators/shift_operations.c",
                       {"shl", "lshr"}, "移位運算 LLVM IR 生成失敗"},
        LLVMIRTestCase{"bitwise_operations", "tests/fixtures/output/operators/bitwise_test.c",
                       {"and", "or", "xor"}, "位元運算 LLVM IR 生成失敗"},
        LLVMIRTestCase{"ternary_operator", "tests/fixtures/output/operators/ternary_test.c",
                       {"icmp", "br", "phi"}, "三元運算子 LLVM IR 生成失敗"},

        // 控制流測試
        LLVMIRTestCase{"if_else", "tests/fixtures/output/control_flow/if_else_test.c",
                       {"icmp", "br", "label"}, "if-else LLVM IR 生成失敗"},
        LLVMIRTestCase{"for_loop", "tests/fixtures/output/control_flow/for_loop_test.c",
                       {"br", "icmp"}, "for 迴圈 LLVM IR 生成失敗"},
        LLVMIRTestCase{"while_loop", "tests/fixtures/output/control_flow/while_loop_test.c",
                       {"br", "icmp"}, "while 迴圈 LLVM IR 生成失敗"},
        LLVMIRTestCase{"do_while_loop", "tests/fixtures/output/control_flow/do_while_test.c",
                       {"br", "icmp"}, "do-while 迴圈 LLVM IR 生成失敗"},

        // Switch 語句測試
        LLVMIRTestCase{"switch_basic", "tests/fixtures/output/control_flow/switch_test.c",
                       {"switch", "label"}, "switch 語句 LLVM IR 生成失敗"},
        LLVMIRTestCase{"switch_fallthrough", "tests/fixtures/output/control_flow/switch_fallthrough_test.c",
                       {"switch", "br"}, "switch fall-through LLVM IR 生成失敗"},
        LLVMIRTestCase{"duff_device", "tests/fixtures/output/control_flow/duff_device_test.c",
                       {"switch", "br", "icmp"}, "達夫裝置 LLVM IR 生成失敗"},

        // Struct 測試
        LLVMIRTestCase{"basic_struct", "tests/fixtures/output/structures/basic_struct_compile.c",
                       {"alloca", "getelementptr", "store", "load"}, "基本 struct LLVM IR 生成失敗"},
        LLVMIRTestCase{"complex_struct", "tests/fixtures/output/structures/complex_struct_compile.c",
                       {"alloca", "getelementptr", "mul"}, "複雜 struct LLVM IR 生成失敗"},
        LLVMIRTestCase{"struct_return_value", "tests/fixtures/output/structures/struct_return_value.c",
                       {"alloca", "load", "store", "add"}, "struct 返回值 LLVM IR 生成失敗"},

        // 短路求值測試
        LLVMIRTestCase{"short_circuit_and", "tests/fixtures/output/operators/short_circuit_test.c",
                       {"br", "label", "phi"}, "AND 短路求值 LLVM IR 生成失敗"},
        LLVMIRTestCase{"short_circuit_or", "tests/fixtures/output/operators/short_circuit_or_test.c",
                       {"br", "label", "phi"}, "OR 短路求值 LLVM IR 生成失敗"},

        // Array 測試
        LLVMIRTestCase{"basic_array", "tests/fixtures/output/arrays/basic_array.c",
                       {"alloca", "getelementptr", "store", "load"}, "基本陣列 LLVM IR 生成失敗"},
        LLVMIRTestCase{"array_initialization", "tests/fixtures/output/arrays/array_initialization.c",
                       {"alloca", "getelementptr", "store"}, "陣列初始化 LLVM IR 生成失敗"},
        LLVMIRTestCase{"multidimensional_array", "tests/fixtures/output/arrays/multidimensional_array.c",
                       {"alloca", "getelementptr"}, "多維陣列 LLVM IR 生成失敗"}
    ),
    LLVMIRTestNameGenerator()
);

// 整合測試:完整編譯流程
TEST_F(OutputTest, CompleteCompilationPipeline) {
    std::string inputFile = "tests/fixtures/output/simple_programs/simple_variable.c";
    std::string llvmFile = test_output_dir + "/simple_variable.ll";
    std::string execFile = test_output_dir + "/simple_variable";

    // 1. 檢查輸入檔案存在
    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;

    // 2. 生成 LLVM IR
    EXPECT_TRUE(generateLLVMIR(inputFile, llvmFile)) << "LLVM IR 生成失敗";
    EXPECT_TRUE(fileExists(llvmFile)) << "LLVM IR 檔案未生成";

    // 3. 編譯可執行檔案
    EXPECT_TRUE(compileFile(inputFile, execFile)) << "可執行檔案編譯失敗";
    EXPECT_TRUE(fileExists(execFile)) << "可執行檔案未生成";

    // 4. 執行並檢查結果
    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 10) << "程式執行結果不正確，期望 10，實際 " << exitCode;
}

// ============================================================================
// 參數化測試：程式執行結果測試
// ============================================================================

class ExecutionResultTest : public OutputTest,
                            public ::testing::WithParamInterface<ExecutionTestCase> {
};

TEST_P(ExecutionResultTest, ProgramExecution) {
    const ExecutionTestCase& testCase = GetParam();
    std::string execFile = test_output_dir + "/" + testCase.testName;

    ASSERT_TRUE(validateAndCompile(testCase.inputFile, execFile));

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, testCase.expectedExitCode)
        << "檔案位置: " << testCase.inputFile << "\n"
        << testCase.description << "，期望 " << testCase.expectedExitCode
        << "，實際 " << exitCode;
}

INSTANTIATE_TEST_SUITE_P(
    AllExecutionTests,
    ExecutionResultTest,
    ::testing::Values(
        // 簡單程式測試
        ExecutionTestCase{"return_constant", "tests/fixtures/output/simple_programs/return_constant.c", 42, "程式返回值不正確"},
        ExecutionTestCase{"return_zero", "tests/fixtures/output/simple_programs/return_zero.c", 0, "程式返回值不正確"},

        // 計算測試
        ExecutionTestCase{"addition", "tests/fixtures/output/calculations/addition.c", 15, "加法計算結果不正確"},
        ExecutionTestCase{"multiplication", "tests/fixtures/output/calculations/multiplication.c", 42, "乘法計算結果不正確"},
        ExecutionTestCase{"complex_arithmetic", "tests/fixtures/output/calculations/complex_arithmetic.c", 15, "複雜算術計算結果不正確"},

        // 運算子測試
        ExecutionTestCase{"shift_operations", "tests/fixtures/output/operators/shift_operations.c", 20, "移位運算結果不正確"},
        ExecutionTestCase{"ternary_test", "tests/fixtures/output/operators/ternary_test.c", 10, "三元運算子結果不正確"},
        ExecutionTestCase{"ternary_nested", "tests/fixtures/output/operators/ternary_nested_test.c", 15, "巢狀三元運算子結果不正確"},
        ExecutionTestCase{"bitwise_test", "tests/fixtures/output/operators/bitwise_test.c", 28, "位元運算結果不正確"},
        ExecutionTestCase{"bitwise_not", "tests/fixtures/output/operators/bitwise_not_test.c", 5, "位元 NOT 運算結果不正確"},
        ExecutionTestCase{"compound_assignment", "tests/fixtures/output/operators/compound_assignment_test.c", 10, "複合賦值運算結果不正確"},

        // 短路求值測試
        ExecutionTestCase{"short_circuit_and", "tests/fixtures/output/operators/short_circuit_test.c", 0, "AND 短路求值結果不正確"},
        ExecutionTestCase{"short_circuit_or", "tests/fixtures/output/operators/short_circuit_or_test.c", 0, "OR 短路求值結果不正確"},
        ExecutionTestCase{"short_circuit_and_eval", "tests/fixtures/output/operators/short_circuit_and_eval_test.c", 1, "AND 求值結果不正確"},
        ExecutionTestCase{"short_circuit_complex", "tests/fixtures/output/operators/short_circuit_complex_test.c", 1, "複雜短路求值結果不正確"},
        ExecutionTestCase{"short_circuit_div_zero", "tests/fixtures/output/operators/short_circuit_div_zero_test.c", 5, "短路求值防除零結果不正確"},

        // 函數測試
        ExecutionTestCase{"simple_function", "tests/fixtures/output/functions/simple_function.c", 15, "函數調用結果不正確"},
        ExecutionTestCase{"recursive_function", "tests/fixtures/output/functions/recursive_function.c", 120, "遞歸函數結果不正確 (5!)"},

        // 控制流測試
        ExecutionTestCase{"if_else", "tests/fixtures/output/control_flow/if_else_test.c", 1, "if-else 結果不正確"},
        ExecutionTestCase{"for_loop", "tests/fixtures/output/control_flow/for_loop_test.c", 15, "for 迴圈結果不正確 (1+2+3+4+5)"},
        ExecutionTestCase{"while_loop", "tests/fixtures/output/control_flow/while_loop_test.c", 10, "while 迴圈結果不正確 (1+2+3+4)"},
        ExecutionTestCase{"do_while", "tests/fixtures/output/control_flow/do_while_test.c", 10, "do-while 迴圈結果不正確 (0+1+2+3+4)"},
        ExecutionTestCase{"break_test", "tests/fixtures/output/control_flow/break_test.c", 10, "break 語句結果不正確 (0+1+2+3+4)"},
        ExecutionTestCase{"continue_test", "tests/fixtures/output/control_flow/continue_test.c", 40, "continue 語句結果不正確 (0+1+2+3+4+6+7+8+9)"},
        ExecutionTestCase{"nested_break_continue", "tests/fixtures/output/control_flow/nested_break_continue_test.c", 90, "巢狀 break/continue 結果不正確"},

        // Goto 測試
        ExecutionTestCase{"goto_simple", "tests/fixtures/output/control_flow/goto_simple_test.c", 5, "簡單 goto 結果不正確"},
        ExecutionTestCase{"goto_loop", "tests/fixtures/output/control_flow/goto_loop_test.c", 10, "goto 循環結果不正確 (0+1+2+3+4)"},
        ExecutionTestCase{"goto_nested", "tests/fixtures/output/control_flow/goto_nested_test.c", 35, "goto 跳出巢狀循環結果不正確"},
        ExecutionTestCase{"goto_multiple_labels", "tests/fixtures/output/control_flow/goto_multiple_labels_test.c", 111, "多標籤 goto 結果不正確"},

        // Switch 測試
        ExecutionTestCase{"switch_basic", "tests/fixtures/output/control_flow/switch_test.c", 30, "switch 語句結果不正確"},
        ExecutionTestCase{"switch_default", "tests/fixtures/output/control_flow/switch_default_test.c", 99, "switch default 結果不正確"},
        ExecutionTestCase{"switch_fallthrough", "tests/fixtures/output/control_flow/switch_fallthrough_test.c", 30, "switch fall-through 結果不正確 (10+20)"},
        ExecutionTestCase{"switch_cascade", "tests/fixtures/output/control_flow/switch_cascade_test.c", 15, "switch cascade 結果不正確 (5+4+3+2+1)"},
        ExecutionTestCase{"duff_device", "tests/fixtures/output/control_flow/duff_device_test.c", 36, "達夫裝置結果不正確 (8+7+6+5+4+3+2+1)"},
        ExecutionTestCase{"switch_no_default", "tests/fixtures/output/control_flow/switch_no_default_test.c", 100, "沒有 default 的 switch 結果不正確"},

        // Struct 測試
        ExecutionTestCase{"basic_struct", "tests/fixtures/output/structures/basic_struct_compile.c", 30, "基本 struct 執行結果不正確 (10+20)"},
        ExecutionTestCase{"struct_size", "tests/fixtures/output/structures/struct_size_test.c", 42, "struct 大小測試執行結果不正確"},
        ExecutionTestCase{"struct_return_value", "tests/fixtures/output/structures/struct_return_value.c", 40, "struct 返回值測試執行結果不正確 (15+25)"},
        ExecutionTestCase{"complex_struct", "tests/fixtures/output/structures/complex_struct_compile.c", 50, "複雜 struct 執行結果不正確 (10*5)"},

        // Array 測試
        ExecutionTestCase{"basic_array", "tests/fixtures/output/arrays/basic_array.c", 60, "基本陣列操作結果不正確 (10+20+30)"},
        ExecutionTestCase{"array_initialization", "tests/fixtures/output/arrays/array_initialization.c", 6, "陣列初始化結果不正確 (1+5)"},
        ExecutionTestCase{"array_loop", "tests/fixtures/output/arrays/array_loop.c", 20, "陣列迴圈操作結果不正確 (0+2+4+6+8)"},
        ExecutionTestCase{"multidimensional_array", "tests/fixtures/output/arrays/multidimensional_array.c", 7, "多維陣列結果不正確 (1+6)"},
        ExecutionTestCase{"array_as_parameter", "tests/fixtures/output/arrays/array_as_parameter.c", 50, "陣列作為參數結果不正確 (5+10+15+20)"},
        ExecutionTestCase{"char_array_string", "tests/fixtures/output/arrays/char_array_string.c", 5, "字元陣列結果不正確 (strlen)"},
        ExecutionTestCase{"array_modify_in_loop", "tests/fixtures/output/arrays/array_modify_in_loop.c", 120, "迴圈中修改陣列結果不正確 (20+40+60)"}
    ),
    ExecutionTestNameGenerator()
);