#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <sys/wait.h>
#include <unistd.h>

namespace fs = std::filesystem;

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
};

// LLVM IR 生成測試
TEST_F(OutputTest, GenerateLLVMIR_ReturnConstant) {
    std::string inputFile = "tests/fixtures/output/simple_programs/return_constant.c";
    std::string llvmFile = test_output_dir + "/return_constant.ll";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    EXPECT_TRUE(generateLLVMIR(inputFile, llvmFile)) << "LLVM IR 生成失敗";
    EXPECT_TRUE(fileExists(llvmFile)) << "LLVM IR 檔案未生成";

    // 檢查 LLVM IR 內容是否包含 main 函數
    EXPECT_TRUE(llvmIRContains(llvmFile, "define")) << "LLVM IR 缺少函數定義";
    EXPECT_TRUE(llvmIRContains(llvmFile, "main")) << "LLVM IR 缺少 main 函數";
}

TEST_F(OutputTest, GenerateLLVMIR_SimpleCalculation) {
    std::string inputFile = "tests/fixtures/output/calculations/addition.c";
    std::string llvmFile = test_output_dir + "/addition.ll";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    EXPECT_TRUE(generateLLVMIR(inputFile, llvmFile)) << "LLVM IR 生成失敗";
    EXPECT_TRUE(fileExists(llvmFile)) << "LLVM IR 檔案未生成";

    // 檢查是否包含算術運算指令
    EXPECT_TRUE(llvmIRContains(llvmFile, "add")) << "LLVM IR 缺少加法指令";
}

TEST_F(OutputTest, GenerateLLVMIR_FunctionCall) {
    std::string inputFile = "tests/fixtures/output/functions/simple_function.c";
    std::string llvmFile = test_output_dir + "/simple_function.ll";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    EXPECT_TRUE(generateLLVMIR(inputFile, llvmFile)) << "LLVM IR 生成失敗";
    EXPECT_TRUE(fileExists(llvmFile)) << "LLVM IR 檔案未生成";

    // 檢查是否包含函數調用
    EXPECT_TRUE(llvmIRContains(llvmFile, "call")) << "LLVM IR 缺少函數調用指令";
}

TEST_F(OutputTest, GenerateLLVMIR_ShiftOperations) {
    std::string inputFile = "tests/fixtures/output/operators/shift_operations.c";
    std::string llvmFile = test_output_dir + "/shift_operations.ll";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    EXPECT_TRUE(generateLLVMIR(inputFile, llvmFile)) << "LLVM IR 生成失敗";
    EXPECT_TRUE(fileExists(llvmFile)) << "LLVM IR 檔案未生成";

    EXPECT_TRUE(llvmIRContains(llvmFile, "shl")) << "LLVM IR 缺少左移指令";
    EXPECT_TRUE(llvmIRContains(llvmFile, "lshr")) << "LLVM IR 缺少右移指令";
}

// 可執行檔案生成測試
TEST_F(OutputTest, GenerateExecutable_ReturnConstant) {
    std::string inputFile = "tests/fixtures/output/simple_programs/return_constant.c";
    std::string execFile = test_output_dir + "/return_constant";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    EXPECT_TRUE(compileFile(inputFile, execFile)) << "可執行檔案編譯失敗";
    EXPECT_TRUE(fileExists(execFile)) << "可執行檔案未生成";
}

TEST_F(OutputTest, GenerateExecutable_AllTestFiles) {
    std::vector<std::pair<std::string, std::string>> testFiles = {
        {"tests/fixtures/output/simple_programs/return_zero.c", "return_zero"},
        {"tests/fixtures/output/simple_programs/simple_variable.c", "simple_variable"},
        {"tests/fixtures/output/calculations/multiplication.c", "multiplication"},
        {"tests/fixtures/output/functions/simple_function.c", "simple_function"},
        {"tests/fixtures/output/operators/shift_operations.c", "shift_operations"}
    };

    for (const auto& testFile : testFiles) {
        std::string inputFile = testFile.first;
        std::string execFile = test_output_dir + "/" + testFile.second;

        ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
        EXPECT_TRUE(compileFile(inputFile, execFile)) << "可執行檔案編譯失敗: " << inputFile;
        EXPECT_TRUE(fileExists(execFile)) << "可執行檔案未生成: " << execFile;
    }
}

// 程式執行結果測試
TEST_F(OutputTest, ExecutionResult_ReturnConstant) {
    std::string inputFile = "tests/fixtures/output/simple_programs/return_constant.c";
    std::string execFile = test_output_dir + "/return_constant";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 42) << "程式返回值不正確，期望 42，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_ReturnZero) {
    std::string inputFile = "tests/fixtures/output/simple_programs/return_zero.c";
    std::string execFile = test_output_dir + "/return_zero";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 0) << "程式返回值不正確，期望 0，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_Addition) {
    std::string inputFile = "tests/fixtures/output/calculations/addition.c";
    std::string execFile = test_output_dir + "/addition";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 15) << "加法計算結果不正確，期望 15，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_Multiplication) {
    std::string inputFile = "tests/fixtures/output/calculations/multiplication.c";
    std::string execFile = test_output_dir + "/multiplication";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 42) << "乘法計算結果不正確，期望 42，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_ComplexArithmetic) {
    std::string inputFile = "tests/fixtures/output/calculations/complex_arithmetic.c";
    std::string execFile = test_output_dir + "/complex_arithmetic";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 15) << "複雜算術計算結果不正確，期望 15，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_ShiftOperations) {
    std::string inputFile = "tests/fixtures/output/operators/shift_operations.c";
    std::string execFile = test_output_dir + "/shift_operations";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 20) << "移位運算結果不正確，期望 20，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_TernaryOperator) {
    std::string inputFile = "tests/fixtures/output/operators/ternary_test.c";
    std::string execFile = test_output_dir + "/ternary_test";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 10) << "三元運算子結果不正確，期望 10，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_TernaryNested) {
    std::string inputFile = "tests/fixtures/output/operators/ternary_nested_test.c";
    std::string execFile = test_output_dir + "/ternary_nested_test";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 15) << "巢狀三元運算子結果不正確，期望 15，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_BitwiseOperations) {
    std::string inputFile = "tests/fixtures/output/operators/bitwise_test.c";
    std::string execFile = test_output_dir + "/bitwise_test";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 28) << "位元運算結果不正確，期望 28，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_BitwiseNot) {
    std::string inputFile = "tests/fixtures/output/operators/bitwise_not_test.c";
    std::string execFile = test_output_dir + "/bitwise_not_test";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 5) << "位元 NOT 運算結果不正確，期望 5，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_CompoundAssignment) {
    std::string inputFile = "tests/fixtures/output/operators/compound_assignment_test.c";
    std::string execFile = test_output_dir + "/compound_assignment_test";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 10) << "複合賦值運算結果不正確，期望 10，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_SimpleFunction) {
    std::string inputFile = "tests/fixtures/output/functions/simple_function.c";
    std::string execFile = test_output_dir + "/simple_function";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 15) << "函數調用結果不正確，期望 15，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_RecursiveFunction) {
    std::string inputFile = "tests/fixtures/output/functions/recursive_function.c";
    std::string execFile = test_output_dir + "/recursive_function";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 120) << "遞歸函數結果不正確，期望 120 (5!)，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_IfElse) {
    std::string inputFile = "tests/fixtures/output/control_flow/if_else_test.c";
    std::string execFile = test_output_dir + "/if_else_test";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 1) << "if-else 結果不正確，期望 1，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_ForLoop) {
    std::string inputFile = "tests/fixtures/output/control_flow/for_loop_test.c";
    std::string execFile = test_output_dir + "/for_loop_test";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 15) << "for 迴圈結果不正確，期望 15 (1+2+3+4+5)，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_WhileLoop) {
    std::string inputFile = "tests/fixtures/output/control_flow/while_loop_test.c";
    std::string execFile = test_output_dir + "/while_loop_test";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 10) << "while 迴圈結果不正確，期望 10 (1+2+3+4)，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_DoWhileLoop) {
    std::string inputFile = "tests/fixtures/output/control_flow/do_while_test.c";
    std::string execFile = test_output_dir + "/do_while_test";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 10) << "do-while 迴圈結果不正確，期望 10 (0+1+2+3+4)，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_Break) {
    std::string inputFile = "tests/fixtures/output/control_flow/break_test.c";
    std::string execFile = test_output_dir + "/break_test";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 10) << "break 語句結果不正確，期望 10 (0+1+2+3+4)，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_Continue) {
    std::string inputFile = "tests/fixtures/output/control_flow/continue_test.c";
    std::string execFile = test_output_dir + "/continue_test";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 40) << "continue 語句結果不正確，期望 40 (0+1+2+3+4+6+7+8+9)，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_NestedBreakContinue) {
    std::string inputFile = "tests/fixtures/output/control_flow/nested_break_continue_test.c";
    std::string execFile = test_output_dir + "/nested_break_continue_test";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 90) << "巢狀 break/continue 結果不正確，期望 90，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_GotoSimple) {
    std::string inputFile = "tests/fixtures/output/control_flow/goto_simple_test.c";
    std::string execFile = test_output_dir + "/goto_simple_test";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 5) << "簡單 goto 結果不正確，期望 5，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_GotoLoop) {
    std::string inputFile = "tests/fixtures/output/control_flow/goto_loop_test.c";
    std::string execFile = test_output_dir + "/goto_loop_test";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 10) << "goto 循環結果不正確，期望 10 (0+1+2+3+4)，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_GotoNested) {
    std::string inputFile = "tests/fixtures/output/control_flow/goto_nested_test.c";
    std::string execFile = test_output_dir + "/goto_nested_test";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 35) << "goto 跳出巢狀循環結果不正確，期望 35，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_GotoMultipleLabels) {
    std::string inputFile = "tests/fixtures/output/control_flow/goto_multiple_labels_test.c";
    std::string execFile = test_output_dir + "/goto_multiple_labels_test";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 111) << "多標籤 goto 結果不正確，期望 111，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_SwitchBasic) {
    std::string inputFile = "tests/fixtures/output/control_flow/switch_test.c";
    std::string execFile = test_output_dir + "/switch_test";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 30) << "switch 語句結果不正確，期望 30，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_SwitchDefault) {
    std::string inputFile = "tests/fixtures/output/control_flow/switch_default_test.c";
    std::string execFile = test_output_dir + "/switch_default_test";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 99) << "switch default 結果不正確，期望 99，實際 " << exitCode;
}

TEST_F(OutputTest, ExecutionResult_SwitchFallthrough) {
    std::string inputFile = "tests/fixtures/output/control_flow/switch_fallthrough_test.c";
    std::string execFile = test_output_dir + "/switch_fallthrough_test";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "編譯失敗";

    int exitCode = executeProgram(execFile);
    EXPECT_EQ(exitCode, 30) << "switch fall-through 結果不正確，期望 30 (10+20)，實際 " << exitCode;
}

// 整合測試：完整編譯流程
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

// Struct 輸出測試
TEST_F(OutputTest, BasicStructCompile) {
    std::string inputFile = "tests/fixtures/output/structures/basic_struct_compile.c";
    std::string execFile = test_output_dir + "/basic_struct_compile";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    EXPECT_TRUE(compileFile(inputFile, execFile)) << "基本 struct 編譯失敗";

    if (fileExists(execFile)) {
        int exitCode = executeProgram(execFile);
        EXPECT_EQ(exitCode, 30) << "基本 struct 執行結果不正確，期望 30 (10+20)，實際 " << exitCode;
    }
}

TEST_F(OutputTest, StructSizeTest) {
    std::string inputFile = "tests/fixtures/output/structures/struct_size_test.c";
    std::string execFile = test_output_dir + "/struct_size_test";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    EXPECT_TRUE(compileFile(inputFile, execFile)) << "struct 大小測試編譯失敗";

    if (fileExists(execFile)) {
        int exitCode = executeProgram(execFile);
        EXPECT_EQ(exitCode, 42) << "struct 大小測試執行結果不正確，期望 42，實際 " << exitCode;
    }
}

TEST_F(OutputTest, StructReturnValue) {
    std::string inputFile = "tests/fixtures/output/structures/struct_return_value.c";
    std::string execFile = test_output_dir + "/struct_return_value";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    EXPECT_TRUE(compileFile(inputFile, execFile)) << "struct 返回值測試編譯失敗";

    if (fileExists(execFile)) {
        int exitCode = executeProgram(execFile);
        EXPECT_EQ(exitCode, 40) << "struct 返回值測試執行結果不正確，期望 40 (15+25)，實際 " << exitCode;
    }
}

TEST_F(OutputTest, ComplexStructCompile) {
    std::string inputFile = "tests/fixtures/output/structures/complex_struct_compile.c";
    std::string execFile = test_output_dir + "/complex_struct_compile";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    EXPECT_TRUE(compileFile(inputFile, execFile)) << "複雜 struct 編譯失敗";

    if (fileExists(execFile)) {
        int exitCode = executeProgram(execFile);
        EXPECT_EQ(exitCode, 50) << "複雜 struct 執行結果不正確，期望 50 (10*5)，實際 " << exitCode;
    }
}

// Struct LLVM IR 生成測試
TEST_F(OutputTest, BasicStructLLVMIR) {
    std::string inputFile = "tests/fixtures/output/structures/basic_struct_compile.c";
    std::string llvmFile = test_output_dir + "/basic_struct.ll";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;
    EXPECT_TRUE(generateLLVMIR(inputFile, llvmFile)) << "基本 struct LLVM IR 生成失敗";
    EXPECT_TRUE(fileExists(llvmFile)) << "LLVM IR 檔案未生成";

    if (fileExists(llvmFile)) {
        EXPECT_TRUE(llvmIRContains(llvmFile, "define")) << "LLVM IR 缺少函數定義";
        EXPECT_TRUE(llvmIRContains(llvmFile, "main")) << "LLVM IR 缺少 main 函數";
    }
}