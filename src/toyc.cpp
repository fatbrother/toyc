#include <iostream>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include "ast/external_definition.hpp"
#include "parser/parse_file.hpp"

extern toyc::ast::NExternalDeclaration *program;

void help() {
    std::cout << "Usage: toyc <filename>" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help     Show this help message" << std::endl;
    std::cout << "  -o, --output   Specify output file" << std::endl;
}

int main(int argc, char *argv[]) {
    int res = 0;
    std::string fileName;

    if (argc < 2) {
        help();
        return -1;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            help();
            return 0;
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                // Handle output file option
                ++i;
            } else {
                std::cerr << "Error: No output file specified." << std::endl;
                return -1;
            }
        } else {
            fileName = arg;
        }
    }
    if (fileName.empty()) {
        std::cerr << "Error: No input file specified." << std::endl;
        return -1;
    }
    // Parse the file
    res = toyc::parser::parseFile(fileName);
    if (res != 0) {
        std::cerr << "Failed to parse file: " << fileName << std::endl;
        return -1;
    }
    std::cout << "File parsed successfully: " << fileName << std::endl;

    llvm::LLVMContext context;
    llvm::Module module("main", context);
    llvm::IRBuilder<> builder(context);

    for (auto &decl = program; decl != nullptr; decl = decl->next) {
        decl->codegen(context, module, builder);
    }

    module.print(llvm::outs(), nullptr);

    return 0;
}