#include <gtest/gtest.h>
#include <sstream>
#include <fstream>
#include "utility/error_handler.hpp"

using namespace toyc::utility;

class ErrorHandlerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 所有測試現在使用 fixtures 資料夾中的檔案
        // 不需要動態創建測試檔案
    }

    void TearDown() override {}
};

TEST_F(ErrorHandlerTest, BasicErrorHandling) {
    ErrorHandler handler("syntax error", 2, 14);  // "int x = 10" 後面遺漏分號
    handler.setFileName("tests/fixtures/error_handler/basic_syntax_error.c");

    std::ostringstream output;
    handler.logError(output);

    std::string result = output.str();

    // 檢查錯誤訊息格式
    EXPECT_TRUE(result.find("tests/fixtures/error_handler/basic_syntax_error.c:2:14: error: syntax error") != std::string::npos);
    EXPECT_TRUE(result.find("int x = 10") != std::string::npos);
    EXPECT_TRUE(result.find("^") != std::string::npos);
}

TEST_F(ErrorHandlerTest, TabHandling) {
    ErrorHandler handler("syntax error", 3, 11);  // "int y = ;" 位置
    handler.setFileName("tests/fixtures/error_handler/tab_syntax_error.c");

    std::ostringstream output;
    handler.logError(output);

    std::string result = output.str();

    // 檢查錯誤訊息包含正確的行
    EXPECT_TRUE(result.find("int y = ;") != std::string::npos);
    EXPECT_TRUE(result.find("^") != std::string::npos);

    // 檢查箭頭位置（應該在正確的視覺位置）
    std::vector<std::string> lines;
    std::stringstream ss(result);
    std::string line;
    while (std::getline(ss, line)) {
        lines.push_back(line);
    }

    // 找到包含 "int y = ;" 的行和箭頭行
    int errorLineIndex = -1;
    int arrowLineIndex = -1;
    for (size_t i = 0; i < lines.size(); i++) {
        if (lines[i].find("int y = ;") != std::string::npos) {
            errorLineIndex = i;
        } else if (lines[i].find("^") != std::string::npos) {
            arrowLineIndex = i;
        }
    }

    EXPECT_NE(errorLineIndex, -1);
    EXPECT_NE(arrowLineIndex, -1);
    EXPECT_EQ(arrowLineIndex, errorLineIndex + 1);
}

TEST_F(ErrorHandlerTest, GetterMethods) {
    ErrorHandler handler("test message", 5, 15);
    handler.setFileName("test.c");

    EXPECT_EQ(handler.getMessage(), "test message");
    EXPECT_EQ(handler.getFileName(), "test.c");
    EXPECT_EQ(handler.getLineNumber(), 5);
    EXPECT_EQ(handler.getColumnNumber(), 15);
}

TEST_F(ErrorHandlerTest, FormattedErrorString) {
    ErrorHandler handler("unexpected token", 2, 14);  // "int x = 10" 後面的位置
    handler.setFileName("tests/fixtures/error_handler/basic_syntax_error.c");

    std::string formatted = handler.getFormattedError();

    EXPECT_TRUE(formatted.find("tests/fixtures/error_handler/basic_syntax_error.c:2:14: error: unexpected token") != std::string::npos);
    EXPECT_TRUE(formatted.find("int x = 10") != std::string::npos);
}

TEST_F(ErrorHandlerTest, NoFileName) {
    ErrorHandler handler("general error", 0, 0);

    std::ostringstream output;
    handler.logError(output);

    std::string result = output.str();
    EXPECT_TRUE(result.find("Error: general error") != std::string::npos);
    // 檢查沒有檔案名稱和行號格式
    EXPECT_TRUE(result.find("Error:") != std::string::npos);
    EXPECT_FALSE(result.find(":") < result.find("Error:"));  // Error: 應該是第一個冒號
}

TEST_F(ErrorHandlerTest, NonExistentFile) {
    ErrorHandler handler("file not found", 1, 1);
    handler.setFileName("nonexistent.c");

    std::ostringstream output;
    handler.logError(output);

    std::string result = output.str();

    // 應該仍然顯示錯誤訊息，即使檔案不存在
    EXPECT_TRUE(result.find("nonexistent.c:1:1: error: file not found") != std::string::npos);
}

TEST_F(ErrorHandlerTest, ZeroLineColumn) {
    ErrorHandler handler("initialization error");

    std::ostringstream output;
    handler.logError(output);

    std::string result = output.str();
    EXPECT_TRUE(result.find("Error: initialization error") != std::string::npos);
}

// 整合測試：測試與 parser 的整合
TEST_F(ErrorHandlerTest, IntegrationWithParser) {
    // 這個測試需要實際的 parser 整合
    // 目前只測試錯誤處理器本身的功能
    ErrorHandler handler("syntax error, unexpected ';'", 2, 14);
    handler.setFileName("tests/fixtures/error_handler/basic_syntax_error.c");

    std::string formatted = handler.getFormattedError();

    // 檢查是否包含所有必要的資訊
    EXPECT_TRUE(formatted.find("tests/fixtures/error_handler/basic_syntax_error.c") != std::string::npos);
    EXPECT_TRUE(formatted.find("2:14") != std::string::npos);
    EXPECT_TRUE(formatted.find("syntax error") != std::string::npos);
    EXPECT_TRUE(formatted.find("int x = 10") != std::string::npos);
    EXPECT_TRUE(formatted.find("^") != std::string::npos);
}// 測試不同的 Tab 寬度
TEST_F(ErrorHandlerTest, DifferentTabWidths) {
    // 測試 tab width = 4
    ErrorHandler handler4("error at position", 3, 11);
    handler4.setFileName("tests/fixtures/error_handler/tab_syntax_error.c");

    std::ostringstream output4;
    handler4.logError(output4);

    std::string result4 = output4.str();
    EXPECT_TRUE(result4.find("int y = ;") != std::string::npos);
    EXPECT_TRUE(result4.find("\t\t        ^") != std::string::npos);

    // 測試 tab width = 8
    ErrorHandler handler8("error at position", 3, 11);
    handler8.setFileName("tests/fixtures/error_handler/tab_syntax_error.c");

    std::ostringstream output8;
    handler8.logError(output8);

    std::string result8 = output8.str();
    EXPECT_TRUE(result8.find("int y = ;") != std::string::npos);
    EXPECT_TRUE(result8.find("^") != std::string::npos);
}

// 測試混合縮排的情況
TEST_F(ErrorHandlerTest, MixedIndentationError) {
    ErrorHandler handler("syntax error", 5, 17);  // "int c = ;" 位置
    handler.setFileName("tests/fixtures/error_handler/mixed_indentation_error.c");

    std::ostringstream output;
    handler.logError(output);

    std::string result = output.str();
    EXPECT_TRUE(result.find("tests/fixtures/error_handler/mixed_indentation_error.c:5:17: error: syntax error") != std::string::npos);
    EXPECT_TRUE(result.find("        int c = ;") != std::string::npos);
    EXPECT_TRUE(result.find("                ^") != std::string::npos);
}

// 測試直接輸出 Tab 到終端機的行為
TEST_F(ErrorHandlerTest, DirectTabOutput) {
    ErrorHandler handler("tab character test", 3, 11);
    handler.setFileName("tests/fixtures/error_handler/tab_syntax_error.c");

    std::ostringstream output;
    handler.logError(output);

    std::string result = output.str();
    EXPECT_TRUE(result.find("\t\tint y = ;") != std::string::npos);
    EXPECT_TRUE(result.find("\t\t        ^") != std::string::npos);
}