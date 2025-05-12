#include <iostream>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <unistd.h>

#include "ast/node.hpp"
#include "obj/object_genner.hpp"
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
    std::string tmpFileChar = "abcdefghijklmnopqrstuvwxyz";
    char flag;
    bool isOutputFile = false;

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

    if (outputFileName.empty()) {
        for (int i = 0; i < tmpFileChar.size(); ++i) {
            outputFileName += tmpFileChar[i];
            if (access(outputFileName.c_str(), F_OK) == -1) {
                break;
            }
        }
    }

    llvm::LLVMContext context;
    llvm::Module module(inputFileName, context);
    llvm::IRBuilder<> builder(context);

    for (auto &decl = program; decl != nullptr; decl = decl->next) {
        decl->codegen(context, module, builder);
    }

    llvm::legacy::PassManager passManager;
    passManager.add(llvm::createPromoteMemoryToRegisterPass());
    passManager.add(llvm::createInstructionNamerPass());
    passManager.add(llvm::createReassociatePass());
    passManager.add(llvm::createGVNPass());
    passManager.add(llvm::createCFGSimplificationPass());
    passManager.run(module);

    // print module
    // module.print(llvm::errs(), nullptr);

    // generate object file
    toyc::obj::ObjectGenner objectGenner;
    isOutputFile = objectGenner.generate(module);
    if (!isOutputFile) {
        std::cerr << "Failed to generate object file." << std::endl;
        return -1;
    }

    return 0;
}