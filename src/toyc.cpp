#include <iostream>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/Utils.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
#include <unistd.h>
#include <vector>
#include <filesystem>

#include "ast/node.hpp"
#include "obj/object_genner.hpp"
#include "utility/parse_file.hpp"
#include "utility/error_handler.hpp"
#include "utility/preprocessor.hpp"
#include "semantic/parser_actions.hpp"

extern toyc::ast::NExternalDeclaration *program;
extern toyc::utility::ErrorHandler *error_handler;
extern toyc::semantic::ParserActions *parser_actions;

#define TMP_FILE_NAME "%%%%TMP%%%%.o"

void help() {
    std::cout << "Usage: toyc <filename>" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -h              Show this help message" << std::endl;
    std::cout << "  -o <filename>   Specify output file" << std::endl;
    std::cout << "  -l              Emit LLVM IR to the specified file" << std::endl;
    std::cout << "  -E              Run only the preprocessor" << std::endl;
    std::cout << "  -D <macro>      Define a macro" << std::endl;
    std::cout << "  -I <path>       Add include path" << std::endl;
}

int main(int argc, char *argv[]) {
    int res = 0;
    std::string inputFileName;
    std::string outputFileName;
    char flag;
    bool isOutputFile = false;
    bool emitLLVM = false;
    bool preprocessOnly = false;
    std::vector<std::pair<std::string, std::string>> macroDefines;
    std::vector<std::string> includePaths;

    if (argc < 2) {
        help();
        return -1;
    }

    while ((flag = getopt(argc, argv, "hlEo:D:I:")) != -1) {
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
            case 'E':
                preprocessOnly = true;
                break;
            case 'D': {
                std::string define = std::string(optarg);
                size_t equalPos = define.find('=');
                if (equalPos != std::string::npos) {
                    macroDefines.push_back({define.substr(0, equalPos), define.substr(equalPos + 1)});
                } else {
                    macroDefines.push_back({define, "1"});
                }
                break;
            }
            case 'I':
                includePaths.push_back(std::string(optarg));
                break;
            case '?':
            default:
                std::cerr << "Unknown option: " << static_cast<char>(flag) << std::endl;
                return -1;
        }
    }
    inputFileName = argv[optind];
    outputFileName = outputFileName.empty() ? inputFileName.substr(0, inputFileName.find_last_of('.')) : outputFileName;

    if (std::filesystem::exists(inputFileName) == false) {
        std::cerr << "Input file does not exist: " << inputFileName << std::endl;
        return -1;
    }

    // Handle preprocessor-only mode
    if (preprocessOnly) {
        toyc::utility::Preprocessor preprocessor;

        // Add user-defined macros
        for (const auto& macro : macroDefines) {
            preprocessor.addPredefinedMacro(macro.first, macro.second);
        }

        // Add include paths
        for (const auto& path : includePaths) {
            preprocessor.addIncludePath(path);
        }

        std::string preprocessedContent = preprocessor.preprocess(inputFileName);
        std::cout << preprocessedContent;
        return 0;
    }

    // Create ASTContext early so TypeManager is available during parsing
    toyc::ast::ASTContext astContext;
    parser_actions = new toyc::semantic::ParserActions(&astContext.getTypeManager());

    // Parse the file with preprocessor
    res = toyc::parser::parseFileWithPreprocessor(inputFileName, macroDefines, includePaths);
    if (res != 0 && error_handler != nullptr) {
        error_handler->setFileName(inputFileName);
        error_handler->logError();
        return -1;
    }

    // Code generation
    for (auto &decl = program; decl != nullptr; decl = decl->next) {
        toyc::ast::CodegenResult result = decl->codegen(astContext);
        if (false == result.isSuccess()) {
            std::cerr << "Error: \n" << result.getErrorMessage() << std::endl;
            return -1;
        }
    }

    // llvm::legacy::PassManager passManager;
    // passManager.add(llvm::createPromoteMemoryToRegisterPass());
    // passManager.add(llvm::createInstructionNamerPass());
    // passManager.add(llvm::createReassociatePass());
    // passManager.add(llvm::createGVNPass());
    // passManager.add(llvm::createCFGSimplificationPass());
    // passManager.run(astContext.module);

    if (emitLLVM) {
        std::error_code EC;
        llvm::raw_fd_ostream llvmFile(outputFileName, EC);
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
    system(command.c_str());

    // Clean up AST memory
    delete program;

    return 0;
}