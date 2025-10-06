#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;

class CompilerErrorTest : public ::testing::Test {
protected:
    void SetUp() override {
        test_output_dir = "tests/error_temp";
        fs::create_directories(test_output_dir);
    }

    void TearDown() override {
        if (fs::exists(test_output_dir)) {
            fs::remove_all(test_output_dir);
        }
    }

    std::string test_output_dir;

    // 輔助方法：編譯檔案並捕獲輸出
    std::pair<int, std::string> compileWithOutput(const std::string& inputFile) {
        std::string outputFile = test_output_dir + "/error_output.txt";
        std::string command = "./toyc " + inputFile + " 2> " + outputFile;
        int result = system(command.c_str());

        // 讀取錯誤輸出
        std::ifstream file(outputFile);
        std::string errorOutput;
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                errorOutput += line + "\n";
            }
        }

        return {WEXITSTATUS(result), errorOutput};
    }

    // 輔助方法：檢查檔案是否存在
    bool fileExists(const std::string& filepath) {
        return fs::exists(filepath);
    }
};

// 錯誤處理測試
TEST_F(CompilerErrorTest, SyntaxError_MissingSemicolon) {
    std::string inputFile = "tests/fixtures/output/error_cases/missing_semicolon.c";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;

    auto [exitCode, errorOutput] = compileWithOutput(inputFile);

    // 編譯應該失敗
    EXPECT_NE(exitCode, 0) << "編譯器應該檢測到語法錯誤";

    // 錯誤輸出應該包含相關信息
    EXPECT_FALSE(errorOutput.empty()) << "編譯器應該輸出錯誤信息";

    // 可以檢查特定的錯誤訊息
    // EXPECT_TRUE(errorOutput.find("syntax error") != std::string::npos) << "錯誤訊息應該提及語法錯誤";
}

TEST_F(CompilerErrorTest, UndefinedFunction) {
    std::string inputFile = "tests/fixtures/output/error_cases/undefined_function.c";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;

    auto [exitCode, errorOutput] = compileWithOutput(inputFile);

    // 編譯可能成功（語法正確），但連結可能失敗
    // 這取決於編譯器的實作
    if (exitCode != 0) {
        EXPECT_FALSE(errorOutput.empty()) << "如果編譯失敗，應該有錯誤信息";
    }
}

TEST_F(CompilerErrorTest, UnmatchedBraces) {
    std::string inputFile = "tests/fixtures/output/error_cases/unmatched_braces.c";

    ASSERT_TRUE(fileExists(inputFile)) << "測試檔案不存在: " << inputFile;

    auto [exitCode, errorOutput] = compileWithOutput(inputFile);

    // 編譯應該失敗
    EXPECT_NE(exitCode, 0) << "編譯器應該檢測到大括號不匹配錯誤";
    EXPECT_FALSE(errorOutput.empty()) << "編譯器應該輸出錯誤信息";
}

TEST_F(CompilerErrorTest, NonExistentFile) {
    std::string inputFile = "non_existent_file.c";

    auto [exitCode, errorOutput] = compileWithOutput(inputFile);

    // 不存在的檔案應該導致編譯失敗
    EXPECT_NE(exitCode, 0) << "不存在的檔案應該導致編譯失敗";
    EXPECT_FALSE(errorOutput.empty()) << "應該有檔案不存在的錯誤訊息";
}