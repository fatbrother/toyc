#include "obj/object_genner.hpp"

#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Host.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>

namespace toyc::obj {

bool ObjectGenner::generate(llvm::Module& module) {
    auto triple = llvm::sys::getDefaultTargetTriple();
    module.setTargetTriple(triple);

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    std::string error;
    auto target = llvm::TargetRegistry::lookupTarget(triple, error);
    if (!target) {
        llvm::errs() << "Failed to lookup target: " << error << "\n";
        return false;
    }

    auto CPU = "generic";
    auto Features = "";

    llvm::TargetOptions opt;
    auto targetMachine = std::unique_ptr<llvm::TargetMachine>(
        target->createTargetMachine(triple, CPU, Features, opt, llvm::Reloc::PIC_));
    module.setDataLayout(targetMachine->createDataLayout());
    module.setTargetTriple(triple);

    // Emit object file
    std::string objFile = "output.o";
    std::error_code EC;
    llvm::raw_fd_ostream dest(objFile, EC, llvm::sys::fs::OF_None);
    if (EC) {
        llvm::errs() << "Could not open object file: " << EC.message() << "\n";
        return false;
    }

    llvm::legacy::PassManager pass;
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CGFT_ObjectFile)) {
        llvm::errs() << "TargetMachine can't emit a file of this type\n";
        return false;
    }
    pass.run(module);
    dest.flush();

    return true;
}

}  // namespace toyc::obj