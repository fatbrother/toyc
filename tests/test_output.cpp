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
};

// 為 ExecutionTestCase 提供輸出運算子,避免顯示亂碼
std::ostream& operator<<(std::ostream& os, const ExecutionTestCase& testCase) {
    os << "ExecutionTestCase{" << testCase.testName << ", " << testCase.inputFile << "}";
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

// 輔助函數：遍歷目錄並收集所有需要 LLVM IR 測試的 .c 文件
std::vector<LLVMIRTestCase> collectLLVMIRTests() {
    std::vector<LLVMIRTestCase> testCases;
    std::string baseDir = "tests/fixtures/output";

    if (!fs::exists(baseDir)) {
        return testCases;
    }

    // 遞歸遍歷目錄
    for (const auto& entry : fs::recursive_directory_iterator(baseDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".c") {
            std::string filePath = entry.path().string();

            // 跳過 error_cases 目錄中的文件
            if (filePath.find("error_cases") != std::string::npos) {
                continue;
            }

            // 生成測試名稱
            std::string relativePath = fs::relative(entry.path(), baseDir).string();
            std::replace(relativePath.begin(), relativePath.end(), '/', '_');
            std::replace(relativePath.begin(), relativePath.end(), '.', '_');
            if (relativePath.size() >= 2 && relativePath.substr(relativePath.size() - 2) == "_c") {
                relativePath = relativePath.substr(0, relativePath.size() - 2);
            }

            // 基本的 LLVM IR 模式檢查（所有文件都應該有這些）
            std::vector<std::string> patterns = {"define", "ret"};

            testCases.push_back({relativePath, filePath, patterns, "LLVM IR 生成測試: " + relativePath});
        }
    }

    // 按測試名稱排序
    std::sort(testCases.begin(), testCases.end(),
              [](const LLVMIRTestCase& a, const LLVMIRTestCase& b) {
                  return a.testName < b.testName;
              });

    return testCases;
}

INSTANTIATE_TEST_SUITE_P(
    AllLLVMIRTests,
    LLVMIRGenerationTest,
    ::testing::ValuesIn(collectLLVMIRTests()),
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
// Qualifier-specific LLVM IR tests
// ============================================================================

TEST_F(OutputTest, VolatileIntGeneratesVolatileIR) {
    std::string inputFile = "tests/fixtures/output/qualifiers/volatile_int.c";
    std::string llvmFile = test_output_dir + "/volatile_int.ll";

    ASSERT_TRUE(fileExists(inputFile)) << "Test file not found: " << inputFile;
    ASSERT_TRUE(generateLLVMIR(inputFile, llvmFile)) << "LLVM IR generation failed";

    EXPECT_TRUE(llvmIRContains(llvmFile, "load volatile"))
        << "volatile int load should emit 'load volatile'";
    EXPECT_TRUE(llvmIRContains(llvmFile, "store volatile"))
        << "volatile int store should emit 'store volatile'";
}

TEST_F(OutputTest, ConstIntReadCompilesAndRuns) {
    std::string inputFile = "tests/fixtures/output/qualifiers/const_int_read.c";
    std::string execFile = test_output_dir + "/const_int_read";

    ASSERT_TRUE(fileExists(inputFile)) << "Test file not found: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "Compilation failed for const int read";

    std::string output = executeProgramWithOutput(execFile);
    EXPECT_EQ(output, "42\n") << "const int x = 42; printf(\"%d\\n\", x); should print 42";
}

TEST_F(OutputTest, ConstPtrReadCompilesAndRuns) {
    std::string inputFile = "tests/fixtures/output/qualifiers/const_ptr_write_through.c";
    std::string execFile = test_output_dir + "/const_ptr_write_through";

    ASSERT_TRUE(fileExists(inputFile)) << "Test file not found: " << inputFile;
    ASSERT_TRUE(compileFile(inputFile, execFile)) << "Compilation failed for const pointer read";

    std::string output = executeProgramWithOutput(execFile);
    EXPECT_EQ(output, "7\n") << "int * const p = &val; printf(\"%d\\n\", *p); should print 7";
}

TEST_F(OutputTest, ConstIntNoVolatileIR) {
    std::string inputFile = "tests/fixtures/output/qualifiers/const_int_read.c";
    std::string llvmFile = test_output_dir + "/const_int_read.ll";

    ASSERT_TRUE(fileExists(inputFile)) << "Test file not found: " << inputFile;
    ASSERT_TRUE(generateLLVMIR(inputFile, llvmFile)) << "LLVM IR generation failed";

    // const does not affect IR — should NOT produce 'volatile'
    EXPECT_FALSE(llvmIRContains(llvmFile, "load volatile"))
        << "const int should not emit volatile loads";
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
        << "toyc 輸出: " << toycOutput
        << "gcc  輸出: " << gccOutput;
}

// 輔助函數：遍歷目錄並收集所有 .c 文件
std::vector<ExecutionTestCase> collectOutputTests() {
    std::vector<ExecutionTestCase> testCases;
    std::string baseDir = "tests/fixtures/output";

    if (!fs::exists(baseDir)) {
        return testCases;
    }

    // 遞歸遍歷目錄
    for (const auto& entry : fs::recursive_directory_iterator(baseDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".c") {
            std::string filePath = entry.path().string();

            // 跳過 error_cases 目錄中的文件
            if (filePath.find("error_cases") != std::string::npos) {
                continue;
            }

            // 生成測試名稱：使用相對路徑並移除 .c 擴展名
            std::string testName = entry.path().stem().string();

            // 如果想要更詳細的測試名稱，可以包含子目錄
            std::string relativePath = fs::relative(entry.path(), baseDir).string();
            // 將路徑分隔符替換為下劃線
            std::replace(relativePath.begin(), relativePath.end(), '/', '_');
            std::replace(relativePath.begin(), relativePath.end(), '.', '_');
            // 移除末尾的 _c
            if (relativePath.size() >= 2 && relativePath.substr(relativePath.size() - 2) == "_c") {
                relativePath = relativePath.substr(0, relativePath.size() - 2);
            }

            testCases.push_back({relativePath, filePath});
        }
    }    // 按測試名稱排序以確保測試順序一致
    std::sort(testCases.begin(), testCases.end(),
              [](const ExecutionTestCase& a, const ExecutionTestCase& b) {
                  return a.testName < b.testName;
              });

    return testCases;
}

INSTANTIATE_TEST_SUITE_P(
    AllExecutionTests,
    ExecutionResultTest,
    ::testing::ValuesIn(collectOutputTests()),
    ExecutionTestNameGenerator()
);