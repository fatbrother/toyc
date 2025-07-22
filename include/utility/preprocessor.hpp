#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <fstream>
#include <sstream>

namespace toyc::utility {

class Preprocessor {
public:
    struct Macro {
        std::string name;
        std::vector<std::string> parameters;
        std::string body;
        bool isFunction;

        Macro() : isFunction(false) {}
        Macro(const std::string& n, const std::string& b)
            : name(n), body(b), isFunction(false) {}
        Macro(const std::string& n, const std::vector<std::string>& p, const std::string& b)
            : name(n), parameters(p), body(b), isFunction(true) {}
    };

    Preprocessor();
    ~Preprocessor();

    // 主要的預處理函數
    std::string preprocess(const std::string& filename);
    std::string preprocessContent(const std::string& content, const std::string& currentFile);

    // 設定包含路徑
    void addIncludePath(const std::string& path);

    // 預定義宏
    void addPredefinedMacro(const std::string& name, const std::string& value);

private:
    // 內部處理函數
    std::string processLine(const std::string& line, const std::string& currentFile, int lineNumber);
    std::string expandMacros(const std::string& text);
    std::string expandObjectMacros(const std::string& text);
    std::string expandFunctionMacro(const Macro& macro, const std::vector<std::string>& args);

    // 指令處理
    void handleDefine(const std::string& line, int lineNumber);
    std::string handleInclude(const std::string& line, const std::string& currentFile, int lineNumber);
    void handleUndef(const std::string& line, int lineNumber);
    void handleIfdef(const std::string& line, int lineNumber);
    void handleIfndef(const std::string& line, int lineNumber);
    void handleIf(const std::string& line, int lineNumber);
    void handleElse(int lineNumber);
    void handleElif(const std::string& line, int lineNumber);
    void handleEndif(int lineNumber);

    // 條件編譯
    bool evaluateCondition(const std::string& condition);
    bool shouldIncludeCode();

    // 工具函數
    std::string findIncludeFile(const std::string& filename, bool isSystemInclude);
    std::vector<std::string> tokenize(const std::string& str);
    std::string trim(const std::string& str);
    bool isValidIdentifier(const std::string& str);

    // 錯誤處理
    void error(const std::string& message, int lineNumber);
    void warning(const std::string& message, int lineNumber);

private:
    std::unordered_map<std::string, Macro> macros_;
    std::vector<std::string> includePaths_;
    std::unordered_set<std::string> includedFiles_;

    // 條件編譯狀態
    struct ConditionalState {
        bool condition;
        bool hasElse;
        bool isActive;
    };
    std::vector<ConditionalState> conditionalStack_;

    // 錯誤報告
    std::string currentFile_;
    int currentLine_;
    bool hasErrors_;
};

} // namespace toyc::utility
