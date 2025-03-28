#include "parser/parse_file.hpp"

#include <fstream>
#include <iostream>
#include <string>

bool toyc::parser::parseFile(const std::string &fileName) {
    std::ifstream file(fileName);
    if (!file) {
        std::cerr << "Failed to open file: " << fileName << std::endl;
        return false;
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    parseContent(content);

    file.close();

    return true;
}

bool toyc::parser::parseContent(const std::string &content) {
    yylineno = 1;
    YY_BUFFER_STATE my_string_buffer = yy_scan_string(content.c_str());
    yy_switch_to_buffer( my_string_buffer);
    int res = yyparse();
    yy_delete_buffer(my_string_buffer);

    if (res != 0) {
        std::cerr << "Failed to parse content" << std::endl;
        return false;
    }
    return true;
}