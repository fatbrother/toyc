#include <iostream>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <unistd.h>

#include "ast/node.hpp"
#include "obj/object_genner.hpp"
#include "utility/parse_file.hpp"
#include "utility/error_handler.hpp"

extern toyc::ast::NExternalDeclaration *program;
extern toyc::utility::ErrorHandler *error_handler;

#define TMP_FILE_NAME "%%%%TMP%%%%.o"

void help() {
    std::cout << "Usage: toyc <filename>" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h, --help     Show this help message" << std::endl;
    std::cout << "  -o, --output   Specify output file" << std::endl;
    std::cout << "  -l, --emit-llvm  Emit LLVM IR to the specified file" << std::endl;
}

int main(int argc, char *argv[]) {
    int res = 0;
    std::string inputFileName;
    std::string outputFileName;
    char flag;
    bool isOutputFile = false;
    bool emitLLVM = false;

    if (argc < 2) {
        help();
        return -1;
    }

    while ((flag = getopt(argc, argv, "hlo:")) != -1) {
        switch (flag) {
            case 'h':
                help();
                return 0;
            case 'o':
                outputFileName = std::string(optarg);
                break;
            case 'l':
                emitLLVM = true;
                break;
            case '?':
            default:
                std::cerr << "Unknown option: " << static_cast<char>(flag) << std::endl;
                return -1;
        }
    }
    inputFileName = argv[optind];
    outputFileName = outputFileName.empty() ? inputFileName.substr(0, inputFileName.find_last_of('.')) : outputFileName;

    if (inputFileName.empty()) {
        std::cerr << "Error: No input file specified." << std::endl;
        return -1;
    }
    // Parse the file
    res = toyc::parser::parseFile(inputFileName);
    if (res != 0 && error_handler != nullptr) {
        error_handler->setFileName(inputFileName);
        error_handler->logError();
        return -1;
    }

    toyc::ast::ASTContext astContext;
    for (auto &decl = program; decl != nullptr; decl = decl->next) {
        decl->codegen(astContext);
    }

    llvm::legacy::PassManager passManager;
    passManager.add(llvm::createPromoteMemoryToRegisterPass());
    passManager.add(llvm::createInstructionNamerPass());
    passManager.add(llvm::createReassociatePass());
    passManager.add(llvm::createGVNPass());
    passManager.add(llvm::createCFGSimplificationPass());
    passManager.run(astContext.module);

    if (emitLLVM) {
        std::string llvmFileName = outputFileName + ".ll";
        std::error_code EC;
        llvm::raw_fd_ostream llvmFile(llvmFileName, EC);
        if (EC) {
            std::cerr << "Error opening file for writing: " << EC.message() << std::endl;
            return -1;
        }
        astContext.module.print(llvmFile, nullptr);
        llvmFile.close();
        return 0;
    }

    // generate object file
    toyc::obj::ObjectGenner objectGenner;
    isOutputFile = objectGenner.generate(astContext.module, TMP_FILE_NAME);
    if (!isOutputFile) {
        std::cerr << "Failed to generate object file." << std::endl;
        return -1;
    }

    // generate executable file
    std::string command = "gcc -o " + outputFileName + " " TMP_FILE_NAME " -lm";
    int ret = system(command.c_str());
    if (ret == -1) {
        std::cerr << "Failed to generate executable file." << std::endl;
        return -1;
    }

    // remove object file
    command = "rm -f " TMP_FILE_NAME;
    ret = system(command.c_str());

    return 0;
}