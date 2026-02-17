#include "utility/preprocessor.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <regex>
#include <sys/stat.h>
#include <unistd.h>

namespace toyc::utility {

// Helper function for C++17 compatibility (starts_with available in C++20)
inline bool starts_with(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

Preprocessor::Preprocessor() : currentLine_(0), hasErrors_(false) {
    // 添加一些預定義的宏
    addPredefinedMacro("__LINE__", "");
    addPredefinedMacro("__FILE__", "");
    addPredefinedMacro("__STDC__", "1");
    addPredefinedMacro("__STDC_VERSION__", "199901L");

    // 添加標準包含路徑
    addIncludePath("/usr/include");
    addIncludePath("/usr/local/include");
}

Preprocessor::~Preprocessor() {}

void Preprocessor::addIncludePath(const std::string& path) {
    includePaths_.push_back(path);
}

void Preprocessor::addPredefinedMacro(const std::string& name, const std::string& value) {
    macros_[name] = Macro(name, value);
}

std::string Preprocessor::preprocess(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        error("Cannot open file: " + filename, 0);
        return "";
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    return preprocessContent(content, filename);
}

std::string Preprocessor::preprocessContent(const std::string& content, const std::string& currentFile) {
    currentFile_ = currentFile;
    currentLine_ = 0;

    std::istringstream stream(content);
    std::string line;
    std::ostringstream result;

    while (std::getline(stream, line)) {
        currentLine_++;

        // 處理行繼續符 (\)
        while (!line.empty() && line.back() == '\\') {
            line.pop_back();
            std::string nextLine;
            if (std::getline(stream, nextLine)) {
                currentLine_++;
                line += nextLine;
            } else {
                break;
            }
        }

        std::string processedLine = processLine(line, currentFile, currentLine_);

        if (!processedLine.empty()) {
            result << processedLine << "\n";
        }
    }

    // 檢查未匹配的條件指令
    if (!conditionalStack_.empty()) {
        error("Missing #endif directive", currentLine_);
    }

    return result.str();
}
std::string Preprocessor::processLine(const std::string& line, const std::string& currentFile, int lineNumber) {
    std::string trimmedLine = trim(line);

    // 空行或註解行
    if (trimmedLine.empty() || starts_with(trimmedLine, "//")) {
        return shouldIncludeCode() ? line : "";
    }

    // 預處理指令
    if (trimmedLine.length() > 0 && trimmedLine[0] == '#') {
        if (trimmedLine.length() == 1) {
            return "";  // 只有 # 符號，忽略
        }

        std::string directive = trimmedLine.substr(1);
        directive = trim(directive);

        if (starts_with(directive, "define")) {
            if (shouldIncludeCode()) {
                handleDefine(directive, lineNumber);
            }
            return "";
        } else if (starts_with(directive, "include")) {
            if (shouldIncludeCode()) {
                return handleInclude(directive, currentFile, lineNumber);
            }
            return "";
        } else if (starts_with(directive, "undef")) {
            if (shouldIncludeCode()) {
                handleUndef(directive, lineNumber);
            }
            return "";
        } else if (starts_with(directive, "ifdef")) {
            handleIfdef(directive, lineNumber);
            return "";
        } else if (starts_with(directive, "ifndef")) {
            handleIfndef(directive, lineNumber);
            return "";
        } else if (starts_with(directive, "if")) {
            // 純 #if 指令 (已經排除 ifdef 和 ifndef)
            handleIf(directive, lineNumber);
            return "";
        } else if (starts_with(directive, "else")) {
            handleElse(lineNumber);
            return "";
        } else if (starts_with(directive, "elif")) {
            handleElif(directive, lineNumber);
            return "";
        } else if (starts_with(directive, "endif")) {
            handleEndif(lineNumber);
            return "";
        }
    }

    // 一般代碼行
    if (shouldIncludeCode()) {
        return expandMacros(line);
    }

    return "";
}

void Preprocessor::handleDefine(const std::string& line, int lineNumber) {
    // 手動解析而不是使用 tokenize，因為函數宏的語法特殊
    std::string trimmedLine = trim(line);
    if (trimmedLine.length() <= 6) {  // "define" = 6 chars
        error("Invalid #define directive", lineNumber);
        return;
    }

    // 跳過 "define" 關鍵字
    std::string content = trim(trimmedLine.substr(6));

    // 找到第一個空白字符或括號來分離宏名稱
    size_t nameEnd = content.find_first_of(" \t(");
    if (nameEnd == std::string::npos) {
        // 只有宏名稱，沒有值
        macros_[content] = Macro(content, "");
        return;
    }

    std::string macroName = content.substr(0, nameEnd);

    // 檢查是否為函數宏
    if (nameEnd < content.length() && content[nameEnd] == '(') {
        // 函數宏 - 找到對應的右括號
        size_t parenEnd = content.find(')', nameEnd);
        if (parenEnd == std::string::npos) {
            error("Invalid function macro definition - missing closing parenthesis", lineNumber);
            return;
        }

        // 提取參數列表
        std::string paramStr = content.substr(nameEnd + 1, parenEnd - nameEnd - 1);
        std::vector<std::string> parameters;

        if (!trim(paramStr).empty()) {
            std::istringstream paramStream(paramStr);
            std::string param;
            while (std::getline(paramStream, param, ',')) {
                parameters.push_back(trim(param));
            }
        }

        // 提取宏體
        std::string body = "";
        if (parenEnd + 1 < content.length()) {
            body = trim(content.substr(parenEnd + 1));
        }

        macros_[macroName] = Macro(macroName, parameters, body);
    } else {
        // 物件宏
        std::string body = "";
        if (nameEnd < content.length()) {
            body = trim(content.substr(nameEnd));
        }

        macros_[macroName] = Macro(macroName, body);
    }
}

std::string Preprocessor::handleInclude(const std::string& line, const std::string& currentFile, int lineNumber) {
    std::regex includeRegex(R"(include\s*[<"](.*?)[>"])");
    std::smatch match;

    if (std::regex_search(line, match, includeRegex)) {
        std::string filename = match[1];
        bool isSystemInclude = line.find('<') != std::string::npos;

        std::string fullPath = findIncludeFile(filename, isSystemInclude);

        if (fullPath.empty()) {
            error("Cannot find include file: " + filename, lineNumber);
            return "";
        }

        // 防止循環包含
        if (includedFiles_.find(fullPath) != includedFiles_.end()) {
            return "";
        }

        includedFiles_.insert(fullPath);

        // 讀取文件內容但不預處理
        std::ifstream file(fullPath);
        if (!file.is_open()) {
            error("Cannot open include file: " + fullPath, lineNumber);
            includedFiles_.erase(fullPath);
            return "";
        }

        std::string rawContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();

        // 使用當前的預處理器狀態處理包含的內容
        std::string processedContent = preprocessContent(rawContent, fullPath);

        includedFiles_.erase(fullPath);

        return processedContent;
    }

    error("Invalid #include directive", lineNumber);
    return "";
}

void Preprocessor::handleUndef(const std::string& line, int lineNumber) {
    std::vector<std::string> tokens = tokenize(line);

    if (tokens.size() != 2) {
        error("Invalid #undef directive", lineNumber);
        return;
    }

    std::string macroName = tokens[1];
    macros_.erase(macroName);
}

void Preprocessor::handleIfdef(const std::string& line, int lineNumber) {
    std::vector<std::string> tokens = tokenize(line);

    if (tokens.size() != 2) {
        error("Invalid #ifdef directive", lineNumber);
        return;
    }

    std::string macroName = tokens[1];
    bool condition = macros_.find(macroName) != macros_.end();

    ConditionalState state;
    state.condition = condition;
    state.hasElse = false;
    state.isActive = shouldIncludeCode() && condition;

    conditionalStack_.push_back(state);
}

void Preprocessor::handleIfndef(const std::string& line, int lineNumber) {
    std::vector<std::string> tokens = tokenize(line);

    if (tokens.size() != 2) {
        error("Invalid #ifndef directive", lineNumber);
        return;
    }

    std::string macroName = tokens[1];
    bool condition = macros_.find(macroName) == macros_.end();

    ConditionalState state;
    state.condition = condition;
    state.hasElse = false;
    state.isActive = shouldIncludeCode() && condition;

    conditionalStack_.push_back(state);
}

void Preprocessor::handleIf(const std::string& line, int lineNumber) {
    if (line.length() <= 2) {
        error("Invalid #if directive", lineNumber);
        return;
    }

    std::string condition = line.substr(2);  // 移除 "if"
    condition = trim(condition);

    bool result = evaluateCondition(condition);

    ConditionalState state;
    state.condition = result;
    state.hasElse = false;
    state.isActive = shouldIncludeCode() && result;

    conditionalStack_.push_back(state);
}

void Preprocessor::handleElse(int lineNumber) {
    if (conditionalStack_.empty()) {
        error("#else without matching #if", lineNumber);
        return;
    }

    ConditionalState& state = conditionalStack_.back();

    if (state.hasElse) {
        error("Multiple #else for the same #if", lineNumber);
        return;
    }

    state.hasElse = true;
    // 檢查父級條件是否為真
    bool parentActive = true;
    if (conditionalStack_.size() > 1) {
        for (size_t i = 0; i < conditionalStack_.size() - 1; ++i) {
            if (!conditionalStack_[i].isActive) {
                parentActive = false;
                break;
            }
        }
    }
    state.isActive = parentActive && !state.condition;
}

void Preprocessor::handleElif(const std::string& line, int lineNumber) {
    if (conditionalStack_.empty()) {
        error("#elif without matching #if", lineNumber);
        return;
    }

    ConditionalState& state = conditionalStack_.back();

    if (state.hasElse) {
        error("#elif after #else", lineNumber);
        return;
    }

    if (!state.condition) {
        if (line.length() <= 4) {
            error("Invalid #elif directive", lineNumber);
            return;
        }

        std::string condition = line.substr(4);  // 移除 "elif"
        condition = trim(condition);

        bool result = evaluateCondition(condition);
        state.condition = result;

        // 檢查父級條件是否為真
        bool parentActive = true;
        if (conditionalStack_.size() > 1) {
            for (size_t i = 0; i < conditionalStack_.size() - 1; ++i) {
                if (!conditionalStack_[i].isActive) {
                    parentActive = false;
                    break;
                }
            }
        }
        state.isActive = parentActive && result;
    } else {
        state.isActive = false;
    }
}
void Preprocessor::handleEndif(int lineNumber) {
    if (conditionalStack_.empty()) {
        error("#endif without matching #if", lineNumber);
        return;
    }

    conditionalStack_.pop_back();
}

bool Preprocessor::evaluateCondition(const std::string& condition) {
    std::string expr = trim(condition);

    // 先展開宏
    expr = expandObjectMacros(expr);

    // 檢查是否為 defined(MACRO) 或 defined MACRO
    std::regex definedRegex(R"(defined\s*\(\s*(\w+)\s*\)|defined\s+(\w+))");
    std::smatch match;

    if (std::regex_search(expr, match, definedRegex)) {
        std::string macroName = match[1].str();
        if (macroName.empty()) {
            macroName = match[2].str();
        }
        return macros_.find(macroName) != macros_.end();
    }

    // 處理簡單的比較表達式 (==, !=, <, >, <=, >=)
    std::regex comparisonRegex(R"((\d+)\s*(==|!=|<=|>=|<|>)\s*(\d+))");
    if (std::regex_search(expr, match, comparisonRegex)) {
        int left = std::stoi(match[1].str());
        std::string op = match[2].str();
        int right = std::stoi(match[3].str());

        if (op == "==")
            return left == right;
        if (op == "!=")
            return left != right;
        if (op == "<")
            return left < right;
        if (op == ">")
            return left > right;
        if (op == "<=")
            return left <= right;
        if (op == ">=")
            return left >= right;
    }

    // 檢查是否為簡單的宏名稱
    if (isValidIdentifier(expr)) {
        return macros_.find(expr) != macros_.end();
    }

    // 簡單的數字比較（0 為假，非0為真）
    try {
        int value = std::stoi(expr);
        return value != 0;
    } catch (...) {
        // 默認為真
        return true;
    }
}

bool Preprocessor::shouldIncludeCode() {
    if (conditionalStack_.empty()) {
        return true;
    }

    // 檢查是否所有條件都為真
    if (std::any_of(conditionalStack_.begin(), conditionalStack_.end(),
                    [](const ConditionalState& state) { return !state.isActive; })) {
        return false;
    }

    return true;
}

std::string Preprocessor::expandObjectMacros(const std::string& text) {
    std::string result = text;

    // 只展開物件宏
    for (const auto& [name, macro] : macros_) {
        if (name == "__LINE__" || name == "__FILE__")
            continue;

        if (!macro.isFunction) {
            // 物件宏
            size_t pos = 0;
            while ((pos = result.find(name, pos)) != std::string::npos) {
                // 確保這是一個完整的標識符
                bool isCompleteIdentifier = true;

                if (pos > 0 && (std::isalnum(result[pos - 1]) || result[pos - 1] == '_')) {
                    isCompleteIdentifier = false;
                }

                if (pos + name.length() < result.length() &&
                    (std::isalnum(result[pos + name.length()]) || result[pos + name.length()] == '_')) {
                    isCompleteIdentifier = false;
                }

                if (isCompleteIdentifier) {
                    result.replace(pos, name.length(), macro.body);
                    pos += macro.body.length();
                } else {
                    pos += name.length();
                }
            }
        }
    }

    return result;
}

std::string Preprocessor::expandFunctionMacro(const Macro& macro, const std::vector<std::string>& args) {
    if (args.size() != macro.parameters.size()) {
        // 參數數量不匹配，返回原始文本
        return macro.name + "(/* parameter mismatch */)";
    }

    std::string result = macro.body;

    // 替換參數
    for (size_t i = 0; i < macro.parameters.size(); ++i) {
        const std::string& paramName = macro.parameters[i];
        std::string argValue = args[i];

        // 先展開參數中的宏
        argValue = expandObjectMacros(argValue);

        size_t pos = 0;
        while ((pos = result.find(paramName, pos)) != std::string::npos) {
            // 確保這是一個完整的標識符
            bool isCompleteIdentifier = true;

            if (pos > 0 && (std::isalnum(result[pos - 1]) || result[pos - 1] == '_')) {
                isCompleteIdentifier = false;
            }

            if (pos + paramName.length() < result.length() &&
                (std::isalnum(result[pos + paramName.length()]) || result[pos + paramName.length()] == '_')) {
                isCompleteIdentifier = false;
            }

            if (isCompleteIdentifier) {
                result.replace(pos, paramName.length(), argValue);
                pos += argValue.length();
            } else {
                pos += paramName.length();
            }
        }
    }

    return result;
}

std::string Preprocessor::expandMacros(const std::string& text) {
    std::string result = text;
    std::string lastResult;
    int maxIterations = 10;  // 防止無限循環
    int iteration = 0;

    do {
        lastResult = result;
        iteration++;

        // 處理 __LINE__ 和 __FILE__ 特殊宏
        size_t pos = 0;
        while ((pos = result.find("__LINE__", pos)) != std::string::npos) {
            result.replace(pos, 8, std::to_string(currentLine_));
            pos += std::to_string(currentLine_).length();
        }

        pos = 0;
        while ((pos = result.find("__FILE__", pos)) != std::string::npos) {
            result.replace(pos, 8, "\"" + currentFile_ + "\"");
            pos += currentFile_.length() + 2;
        }

        // 展開物件宏
        for (const auto& [name, macro] : macros_) {
            if (name == "__LINE__" || name == "__FILE__")
                continue;

            if (!macro.isFunction) {
                // 物件宏
                pos = 0;
                while ((pos = result.find(name, pos)) != std::string::npos) {
                    // 確保這是一個完整的標識符
                    bool isCompleteIdentifier = true;

                    if (pos > 0 && (std::isalnum(result[pos - 1]) || result[pos - 1] == '_')) {
                        isCompleteIdentifier = false;
                    }

                    if (pos + name.length() < result.length() &&
                        (std::isalnum(result[pos + name.length()]) || result[pos + name.length()] == '_')) {
                        isCompleteIdentifier = false;
                    }

                    if (isCompleteIdentifier) {
                        result.replace(pos, name.length(), macro.body);
                        pos += macro.body.length();
                    } else {
                        pos += name.length();
                    }
                }
            }
        }

        // 展開函數宏
        for (const auto& [name, macro] : macros_) {
            if (name == "__LINE__" || name == "__FILE__")
                continue;

            if (macro.isFunction) {
                // 函數宏的展開
                std::regex functionMacroRegex(name + R"(\s*\(\s*([^)]*)\s*\))");
                std::smatch match;

                pos = 0;
                while (pos < result.length()) {
                    std::string tempResult = result.substr(pos);
                    if (std::regex_search(tempResult, match, functionMacroRegex)) {
                        std::string args = match[1].str();
                        std::vector<std::string> arguments;

                        // 解析參數
                        if (!args.empty()) {
                            std::istringstream argStream(args);
                            std::string arg;
                            while (std::getline(argStream, arg, ',')) {
                                arguments.push_back(trim(arg));
                            }
                        }

                        // 展開函數宏
                        std::string expanded = expandFunctionMacro(macro, arguments);

                        size_t matchPos = pos + match.position();
                        result.replace(matchPos, match.length(), expanded);
                        pos = matchPos + expanded.length();
                    } else {
                        break;
                    }
                }
            }
        }
    } while (result != lastResult && iteration < maxIterations);

    return result;
}

std::string Preprocessor::findIncludeFile(const std::string& filename, bool isSystemInclude) {
    // 輔助函數：檢查文件是否存在
    auto fileExists = [](const std::string& path) -> bool {
        struct stat buffer;
        return (stat(path.c_str(), &buffer) == 0);
    };

    // 輔助函數：獲取目錄路徑
    auto getDirectoryPath = [](const std::string& filePath) -> std::string {
        size_t lastSlash = filePath.find_last_of('/');
        if (lastSlash != std::string::npos) {
            return filePath.substr(0, lastSlash);
        }
        return ".";
    };

    if (!isSystemInclude) {
        // 首先在當前目錄尋找
        std::string currentDir = getDirectoryPath(currentFile_);
        std::string filePath = currentDir + "/" + filename;

        if (fileExists(filePath)) {
            return filePath;
        }
    }

    // 在包含路徑中尋找
    for (const auto& includePath : includePaths_) {
        std::string filePath = includePath + "/" + filename;

        if (fileExists(filePath)) {
            return filePath;
        }
    }

    return "";
}

std::vector<std::string> Preprocessor::tokenize(const std::string& str) {
    std::vector<std::string> tokens;
    std::istringstream stream(str);
    std::string token;

    while (stream >> token) {
        tokens.push_back(token);
    }

    return tokens;
}

std::string Preprocessor::trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }

    size_t end = str.find_last_not_of(" \t\r\n");
    if (end == std::string::npos) {
        return "";
    }

    return str.substr(start, end - start + 1);
}
bool Preprocessor::isValidIdentifier(const std::string& str) {
    if (str.empty() || (!std::isalpha(str[0]) && str[0] != '_')) {
        return false;
    }

    for (char c : str) {
        if (!std::isalnum(c) && c != '_') {
            return false;
        }
    }

    return true;
}

void Preprocessor::error(const std::string& message, int lineNumber) {
    std::cerr << currentFile_ << ":" << lineNumber << ": error: " << message << std::endl;
    hasErrors_ = true;
}

void Preprocessor::warning(const std::string& message, int lineNumber) {
    std::cerr << currentFile_ << ":" << lineNumber << ": warning: " << message << std::endl;
}

}  // namespace toyc::utility
