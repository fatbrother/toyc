#include <iostream>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <unistd.h>

#include "ast/node.hpp"
#include "utility/parse_file.hpp"

extern toyc::ast::NExternalDeclaration *program;

void help() {
    std::cout << "Usage: toyc <filename>" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help     Show this help message" << std::endl;
    std::cout << "  -o, --output   Specify output file" << std::endl;
}

int main(int argc, char *argv[]) {
    int res = 0;
    std::string inputFileName;
    std::string outputFileName;
    char flag;

    if (argc < 2) {
        help();
        return -1;
    }

    while ((flag = getopt(argc, argv, "ho:")) != -1) {
        switch (flag) {
            case 'h':
                help();
                return 0;
            case 'o':
                outputFileName = std::string(optarg);
                break;
            case '?':
            default:
                std::cerr << "Unknown option: " << static_cast<char>(flag) << std::endl;
                return -1;
        }
    }
    inputFileName = argv[optind];

    if (inputFileName.empty()) {
        std::cerr << "Error: No input file specified." << std::endl;
        return -1;
    }
    // Parse the file
    res = toyc::parser::parseFile(inputFileName);
    if (res != 0) {
        std::cerr << "Failed to parse file: " << inputFileName << std::endl;
        return -1;
    }

    llvm::LLVMContext context;
    llvm::Module module(inputFileName, context);
    llvm::IRBuilder<> builder(context);

    for (auto &decl = program; decl != nullptr; decl = decl->next) {
        decl->codegen(context, module, builder);
    }

    module.print(llvm::outs(), nullptr);

    return 0;
}