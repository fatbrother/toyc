#pragma once

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

namespace toyc::utility {

class ErrorHandler {
public:
    ErrorHandler(const std::string &message, int line = 0, int column = 0)
        : errorMessage(message), lineNumber(line), columnNumber(column) {}

    void setFileName(const std::string &name) {
        fileName = name;
    }

    void logError() const {
        logError(std::cerr);
    }

    void logError(std::ostream& output) const {
        std::string errorLine;
        bool fileFound = false;

        // 嘗試讀取錯誤行
        if (!fileName.empty()) {
            std::ifstream file(fileName);
            if (file.is_open()) {
                std::string line;
                int currentLine = 0;
                while (std::getline(file, line)) {
                    currentLine++;
                    if (currentLine == lineNumber) {
                        errorLine = line;
                        fileFound = true;
                        break;
                    }
                }
                file.close();
            }
        }

        // 輸出錯誤資訊標頭
        if (!fileName.empty()) {
            output << fileName;
            if (lineNumber > 0) {
                output << ":" << lineNumber;
                if (columnNumber > 0) {
                    output << ":" << columnNumber;
                }
            }
            output << ": error: " << errorMessage << std::endl;
        } else {
            output << "Error: " << errorMessage << std::endl;
        }

        // 如果有錯誤行，顯示錯誤位置
        if (fileFound && !errorLine.empty()) {
            output << errorLine << std::endl;

            // 直接使用原始字符構建指示行，讓終端機自然處理 Tab
            std::string indicator = buildIndicatorLine(errorLine, columnNumber);
            output << indicator << "^" << std::endl;
        }
    }

    std::string getFormattedError() const {
        std::ostringstream oss;
        logError(oss);
        return oss.str();
    }

    // Getter 方法
    const std::string& getMessage() const { return errorMessage; }
    const std::string& getFileName() const { return fileName; }
    int getLineNumber() const { return lineNumber; }
    int getColumnNumber() const { return columnNumber; }

private:
    std::string errorMessage;
    std::string fileName;
    int lineNumber = 0;
    int columnNumber = 0;
    int tabCount = 0;

    // 構建錯誤指示行，保持原始的 Tab 字符
    std::string buildIndicatorLine(const std::string& line, int logicalColumn) const {
        std::string indicator;
        int logicalCol = 1;

        for (size_t i = 0; i < line.length() && logicalCol < logicalColumn; i++) {
            char c = line[i];

            if (c == '\t') {
                // 直接添加 Tab 字符，讓終端機處理
                indicator += '\t';
            } else {
                // 對於其他字符，添加空格
                indicator += ' ';
            }
            logicalCol++;
        }

        return indicator;
    }
};

} // namespace toyc::utility