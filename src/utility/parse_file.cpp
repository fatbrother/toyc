#include "utility/parse_file.hpp"
#include "utility/preprocessor.hpp"

#include <fstream>
#include <iostream>
#include <string>

typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern int yyparse();
extern int yylineno;
extern YY_BUFFER_STATE yy_scan_string(const char * str);
extern YY_BUFFER_STATE yy_switch_to_buffer(YY_BUFFER_STATE buffer);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

int toyc::parser::parseFile(const std::string &fileName) {
    std::ifstream file(fileName);
    std::string content;

    if (!file) {
        std::cerr << "Failed to open file: " << fileName << std::endl;
        return -1;
    }
    content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    return parseContent(content);
}

int toyc::parser::parseContent(const std::string &content) {
    yylineno = 1;
    YY_BUFFER_STATE my_string_buffer = yy_scan_string(content.c_str());
    yy_switch_to_buffer( my_string_buffer);
    int res = yyparse();
    yy_delete_buffer(my_string_buffer);

    return res;
}

int toyc::parser::parseFileWithPreprocessor(const std::string &fileName,
                                           const std::vector<std::pair<std::string, std::string>>& macros,
                                           const std::vector<std::string>& includePaths) {
    toyc::utility::Preprocessor preprocessor;

    // 添加預定義宏
    preprocessor.addPredefinedMacro("__TOYC__", "1");

    // 添加用戶定義的宏
    for (const auto& macro : macros) {
        preprocessor.addPredefinedMacro(macro.first, macro.second);
    }

    // 添加包含路徑
    for (const auto& path : includePaths) {
        preprocessor.addIncludePath(path);
    }

    std::string preprocessedContent = preprocessor.preprocess(fileName);

    if (preprocessedContent.empty()) {
        std::cerr << "Preprocessing failed for file: " << fileName << std::endl;
        return -1;
    }

    return parseContent(preprocessedContent);
}