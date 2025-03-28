#include <iostream>
#include "parser/parse_file.hpp"

int main() {
    std::string fileName = "test.c";
    toyc::parser::parseFile(fileName);

    return 0;
}