#include "utility/error_handler.hpp"

#include <fstream>
#include <sstream>

namespace toyc::utility {

void ErrorHandler::setFileName(const std::string& name) {
    fileName = name;
}

void ErrorHandler::logError() const {
    logError(std::cerr);
}

void ErrorHandler::logError(std::ostream& output) const {
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
        output << indicator << "^";
        for (int i = 1; i < tokenSize; ++i) {
            output << "~";  // 使用 ~ 符號表示錯誤範圍
        }
        output << std::endl;
    }
}

std::string ErrorHandler::getFormattedError() const {
    std::ostringstream oss;
    logError(oss);
    return oss.str();
}

std::string ErrorHandler::buildIndicatorLine(const std::string& line, int logicalColumn) const {
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

}  // namespace toyc::utility
