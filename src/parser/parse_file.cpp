#include "parser/parse_file.hpp"

#include <fstream>
#include <iostream>
#include <string>

int toyc::parser::parseFile(const std::string &fileName) {
    std::ifstream file(fileName);
    std::string content;

    if (!file) {
        std::cerr << "Failed to open file: " << fileName << std::endl;
        return -1;
    }
    content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    // dump the content to the console
    std::cout << "Content of the file: " << std::endl;
    std::cout << content << std::endl;

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