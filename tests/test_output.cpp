#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <sys/wait.h>
#include <unistd.h>
#include <algorithm>

namespace fs = std::filesystem;

// 執行結果測試的參數化結構
struct ExecutionTestCase {
    std::string testName;
    std::string inputFile;
    std::string expectedOutput;  // 改為預期輸出
    std::string description;
};

// 為 ExecutionTestCase 提供輸出運算子,避免顯示亂碼
std::ostream& operator<<(std::ostream& os, const ExecutionTestCase& testCase) {
    os << "ExecutionTestCase{" << testCase.testName << ", " << testCase.inputFile << ", output=" << testCase.expectedOutput << "}";
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

    // 輔助方法:執行程式並返回退出碼
    int executeProgram(const std::string& executable) {
        int result = system(executable.c_str());
        return WEXITSTATUS(result);
    }

    // 輔助方法:執行程式並捕獲輸出
    std::string executeProgramWithOutput(const std::string& executable) {
        std::string outputFile = test_output_dir + "/exec_output.txt";
        std::string command = executable + " > " + outputFile + " 2>&1";
        system(command.c_str());
        return readFile(outputFile);
    }

    // 輔助方法:使用 gcc 編譯並執行,捕獲輸出 (帶快取)
    std::string compileAndRunWithGCC(const std::string& inputFile) {
        // 建立快取目錄
        std::string cacheDir = "tests/gcc_output_cache";
        fs::create_directories(cacheDir);

        // 根據輸入檔案路徑生成快取檔案名稱
        std::string cacheFileName = inputFile;
        // 將路徑分隔符和擴展名替換為安全字符
        std::replace(cacheFileName.begin(), cacheFileName.end(), '/', '_');
        std::replace(cacheFileName.begin(), cacheFileName.end(), '.', '_');
        std::string cacheFile = cacheDir + "/" + cacheFileName + ".expected";

        // 如果快取存在,直接讀取
        if (fs::exists(cacheFile)) {
            return readFile(cacheFile);
        }

        // 快取不存在,執行 gcc 編譯並運行
        std::string gccExec = test_output_dir + "/gcc_exec";
        std::string outputFile = test_output_dir + "/gcc_output.txt";

        // 使用 gcc 編譯
        std::string compileCmd = "gcc -o " + gccExec + " " + inputFile + " 2>/dev/null";
        int compileResult = system(compileCmd.c_str());

        if (WEXITSTATUS(compileResult) != 0) {
            return "";  // 編譯失敗
        }

        // 執行並捕獲輸出
        std::string runCmd = gccExec + " > " + outputFile + " 2>&1";
        system(runCmd.c_str());

        std::string output = readFile(outputFile);

        // 將輸出存入快取
        std::ofstream cache(cacheFile);
        if (cache.is_open()) {
            cache << output;
            cache.close();
        }

        return output;
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
        LLVMIRTestCase{"sizeof_operator", "tests/fixtures/output/operators/sizeof_test.c",
                       {"alloca", "store"}, "sizeof 運算符 LLVM IR 生成失敗"},

        // 型別轉換測試（拆分後）
        LLVMIRTestCase{"cast_int_to_int", "tests/fixtures/output/operators/cast_int_to_int.c",
                       {"trunc", "sext"}, "整數間轉換 LLVM IR 生成失敗"},
        LLVMIRTestCase{"cast_float_to_int", "tests/fixtures/output/operators/cast_float_to_int.c",
                       {"fptosi"}, "浮點數到整數轉換 LLVM IR 生成失敗"},
        LLVMIRTestCase{"cast_int_to_float", "tests/fixtures/output/operators/cast_int_to_float.c",
                       {"sitofp", "fptosi"}, "整數到浮點數轉換 LLVM IR 生成失敗"},
        LLVMIRTestCase{"cast_division", "tests/fixtures/output/operators/cast_division.c",
                       {"sitofp", "fdiv", "fptosi"}, "除法運算轉換 LLVM IR 生成失敗"},
        LLVMIRTestCase{"cast_char_ascii", "tests/fixtures/output/operators/cast_char_ascii.c",
                       {"sext", "trunc"}, "字元 ASCII 轉換 LLVM IR 生成失敗"},
        LLVMIRTestCase{"cast_negative", "tests/fixtures/output/operators/cast_negative.c",
                       {"sitofp", "fptosi"}, "負數轉換 LLVM IR 生成失敗"},
        LLVMIRTestCase{"cast_expressions", "tests/fixtures/output/operators/cast_expressions.c",
                       {"sitofp", "fadd", "fdiv", "fptosi"}, "表達式中轉換 LLVM IR 生成失敗"},
        LLVMIRTestCase{"cast_multiple", "tests/fixtures/output/operators/cast_multiple.c",
                       {"fptrunc", "fptosi", "sitofp"}, "多重轉換 LLVM IR 生成失敗"},

        LLVMIRTestCase{"comma_operator", "tests/fixtures/output/operators/comma_test.c",
                       {"store", "load"}, "逗號運算符 LLVM IR 生成失敗"},

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
    EXPECT_EQ(exitCode, 0) << "程式執行結果不正確，期望 0，實際 " << exitCode;
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

    // 執行 toyc 編譯的程式並獲取輸出
    std::string toycOutput = executeProgramWithOutput(execFile);

    // 使用 gcc 編譯並執行,獲取輸出
    std::string gccOutput = compileAndRunWithGCC(testCase.inputFile);

    // 確保 gcc 編譯成功
    ASSERT_FALSE(gccOutput.empty())
        << "GCC 編譯失敗: " << testCase.inputFile;

    // 比對兩者輸出是否一致
    EXPECT_EQ(toycOutput, gccOutput)
        << "檔案位置: " << testCase.inputFile << "\n"
        << testCase.description << "\n"
        << "toyc 輸出: " << toycOutput
        << "gcc  輸出: " << gccOutput
        << "預期輸出: " << testCase.expectedOutput;
}

INSTANTIATE_TEST_SUITE_P(
    AllExecutionTests,
    ExecutionResultTest,
    ::testing::Values(
        // 簡單程式測試
        ExecutionTestCase{"return_constant", "tests/fixtures/output/simple_programs/return_constant.c", "42\n", "程式輸出不正確"},
        ExecutionTestCase{"return_zero", "tests/fixtures/output/simple_programs/return_zero.c", "0\n", "程式輸出不正確"},

        // 計算測試
        ExecutionTestCase{"addition", "tests/fixtures/output/calculations/addition.c", "15\n", "加法計算結果不正確"},
        ExecutionTestCase{"multiplication", "tests/fixtures/output/calculations/multiplication.c", "42\n", "乘法計算結果不正確"},
        ExecutionTestCase{"complex_arithmetic", "tests/fixtures/output/calculations/complex_arithmetic.c", "15\n", "複雜算術計算結果不正確"},

        // 運算子測試
        ExecutionTestCase{"shift_operations", "tests/fixtures/output/operators/shift_operations.c", "20\n", "移位運算結果不正確"},
        ExecutionTestCase{"ternary_test", "tests/fixtures/output/operators/ternary_test.c", "10\n", "三元運算子結果不正確"},
        ExecutionTestCase{"ternary_nested", "tests/fixtures/output/operators/ternary_nested_test.c", "15\n", "巢狀三元運算子結果不正確"},
        ExecutionTestCase{"bitwise_test", "tests/fixtures/output/operators/bitwise_test.c", "28\n", "位元運算結果不正確"},
        ExecutionTestCase{"bitwise_not", "tests/fixtures/output/operators/bitwise_not_test.c", "5\n", "位元 NOT 運算結果不正確"},
        ExecutionTestCase{"compound_assignment", "tests/fixtures/output/operators/compound_assignment_test.c", "10\n", "複合賦值運算結果不正確"},
        ExecutionTestCase{"sizeof_test", "tests/fixtures/output/operators/sizeof_test.c", "90\n", "sizeof 運算符結果不正確"},

        // 型別轉換測試（拆分後）
        ExecutionTestCase{"cast_int_to_int", "tests/fixtures/output/operators/cast_int_to_int.c", "644\n", "整數間轉換結果不正確"},
        ExecutionTestCase{"cast_float_to_int", "tests/fixtures/output/operators/cast_float_to_int.c", "12\n", "浮點數到整數轉換結果不正確"},
        ExecutionTestCase{"cast_int_to_float", "tests/fixtures/output/operators/cast_int_to_float.c", "42\n", "整數到浮點數轉換結果不正確"},
        ExecutionTestCase{"cast_division", "tests/fixtures/output/operators/cast_division.c", "4\n", "除法運算轉換結果不正確"},
        ExecutionTestCase{"cast_char_ascii", "tests/fixtures/output/operators/cast_char_ascii.c", "131\n", "字元 ASCII 轉換結果不正確"},
        ExecutionTestCase{"cast_negative", "tests/fixtures/output/operators/cast_negative.c", "246\n", "負數轉換結果不正確 (-10 的無符號表示)"},
        ExecutionTestCase{"cast_expressions", "tests/fixtures/output/operators/cast_expressions.c", "6\n", "表達式中轉換結果不正確"},
        ExecutionTestCase{"cast_multiple", "tests/fixtures/output/operators/cast_multiple.c", "3\n", "多重轉換結果不正確"},

        ExecutionTestCase{"comma_test", "tests/fixtures/output/operators/comma_test.c", "325\n", "逗號運算符結果不正確"},

        // 短路求值測試
        ExecutionTestCase{"short_circuit_and", "tests/fixtures/output/operators/short_circuit_test.c", "0\n", "AND 短路求值結果不正確"},
        ExecutionTestCase{"short_circuit_or", "tests/fixtures/output/operators/short_circuit_or_test.c", "0\n", "OR 短路求值結果不正確"},
        ExecutionTestCase{"short_circuit_and_eval", "tests/fixtures/output/operators/short_circuit_and_eval_test.c", "1\n", "AND 求值結果不正確"},
        ExecutionTestCase{"short_circuit_complex", "tests/fixtures/output/operators/short_circuit_complex_test.c", "1\n", "複雜短路求值結果不正確"},
        ExecutionTestCase{"short_circuit_div_zero", "tests/fixtures/output/operators/short_circuit_div_zero_test.c", "5\n", "短路求值防除零結果不正確"},

        // 函數測試
        ExecutionTestCase{"simple_function", "tests/fixtures/output/functions/simple_function.c", "15\n", "函數調用結果不正確"},
        ExecutionTestCase{"recursive_function", "tests/fixtures/output/functions/recursive_function.c", "120\n", "遞歸函數結果不正確 (5!)"},

        // 控制流測試
        ExecutionTestCase{"if_else", "tests/fixtures/output/control_flow/if_else_test.c", "1\n", "if-else 結果不正確"},
        ExecutionTestCase{"for_loop", "tests/fixtures/output/control_flow/for_loop_test.c", "15\n", "for 迴圈結果不正確 (1+2+3+4+5)"},
        ExecutionTestCase{"while_loop", "tests/fixtures/output/control_flow/while_loop_test.c", "10\n", "while 迴圈結果不正確 (1+2+3+4)"},
        ExecutionTestCase{"do_while", "tests/fixtures/output/control_flow/do_while_test.c", "10\n", "do-while 迴圈結果不正確 (0+1+2+3+4)"},
        ExecutionTestCase{"break_test", "tests/fixtures/output/control_flow/break_test.c", "10\n", "break 語句結果不正確 (0+1+2+3+4)"},
        ExecutionTestCase{"continue_test", "tests/fixtures/output/control_flow/continue_test.c", "40\n", "continue 語句結果不正確 (0+1+2+3+4+6+7+8+9)"},
        ExecutionTestCase{"nested_break_continue", "tests/fixtures/output/control_flow/nested_break_continue_test.c", "90\n", "巢狀 break/continue 結果不正確"},

        // Goto 測試
        ExecutionTestCase{"goto_simple", "tests/fixtures/output/control_flow/goto_simple_test.c", "5\n", "簡單 goto 結果不正確"},
        ExecutionTestCase{"goto_loop", "tests/fixtures/output/control_flow/goto_loop_test.c", "10\n", "goto 循環結果不正確 (0+1+2+3+4)"},
        ExecutionTestCase{"goto_nested", "tests/fixtures/output/control_flow/goto_nested_test.c", "35\n", "goto 跳出巢狀循環結果不正確"},
        ExecutionTestCase{"goto_multiple_labels", "tests/fixtures/output/control_flow/goto_multiple_labels_test.c", "111\n", "多標籤 goto 結果不正確"},

        // Switch 測試
        ExecutionTestCase{"switch_basic", "tests/fixtures/output/control_flow/switch_test.c", "30\n", "switch 語句結果不正確"},
        ExecutionTestCase{"switch_default", "tests/fixtures/output/control_flow/switch_default_test.c", "99\n", "switch default 結果不正確"},
        ExecutionTestCase{"switch_fallthrough", "tests/fixtures/output/control_flow/switch_fallthrough_test.c", "30\n", "switch fall-through 結果不正確 (10+20)"},
        ExecutionTestCase{"switch_cascade", "tests/fixtures/output/control_flow/switch_cascade_test.c", "15\n", "switch cascade 結果不正確 (5+4+3+2+1)"},
        ExecutionTestCase{"duff_device", "tests/fixtures/output/control_flow/duff_device_test.c", "36\n", "達夫裝置結果不正確 (8+7+6+5+4+3+2+1)"},
        ExecutionTestCase{"switch_no_default", "tests/fixtures/output/control_flow/switch_no_default_test.c", "100\n", "沒有 default 的 switch 結果不正確"},

        // Struct 測試
        ExecutionTestCase{"basic_struct", "tests/fixtures/output/structures/basic_struct_compile.c", "30\n", "基本 struct 執行結果不正確 (10+20)"},
        ExecutionTestCase{"struct_size", "tests/fixtures/output/structures/struct_size_test.c", "42\n", "struct 大小測試執行結果不正確"},
        ExecutionTestCase{"struct_return_value", "tests/fixtures/output/structures/struct_return_value.c", "40\n", "struct 返回值測試執行結果不正確 (15+25)"},
        ExecutionTestCase{"complex_struct", "tests/fixtures/output/structures/complex_struct_compile.c", "50\n", "複雜 struct 執行結果不正確 (10*5)"},

        // Array 測試
        ExecutionTestCase{"basic_array", "tests/fixtures/output/arrays/basic_array.c", "60\n", "基本陣列操作結果不正確 (10+20+30)"},
        ExecutionTestCase{"array_initialization", "tests/fixtures/output/arrays/array_initialization.c", "6\n", "陣列初始化結果不正確 (1+5)"},
        ExecutionTestCase{"array_loop", "tests/fixtures/output/arrays/array_loop.c", "20\n", "陣列迴圈操作結果不正確 (0+2+4+6+8)"},
        ExecutionTestCase{"multidimensional_array", "tests/fixtures/output/arrays/multidimensional_array.c", "7\n", "多維陣列結果不正確 (1+6)"},
        ExecutionTestCase{"array_as_parameter", "tests/fixtures/output/arrays/array_as_parameter.c", "50\n", "陣列作為參數結果不正確 (5+10+15+20)"},
        ExecutionTestCase{"char_array_string", "tests/fixtures/output/arrays/char_array_string.c", "5\n", "字元陣列結果不正確 (strlen)"},
        ExecutionTestCase{"array_modify_in_loop", "tests/fixtures/output/arrays/array_modify_in_loop.c", "120\n", "迴圈中修改陣列結果不正確 (20+40+60)"}
    ),
    ExecutionTestNameGenerator()
);