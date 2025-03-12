#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <iostream>

int main() {
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> module = std::make_unique<llvm::Module>("my_module", context);
    llvm::IRBuilder<> builder(context);
    llvm::FunctionType* funcType = llvm::FunctionType::get(builder.getInt32Ty(), false);
    llvm::Function* mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module.get());
    llvm::BasicBlock* entry = llvm::BasicBlock::Create(context, "entry", mainFunc);

    builder.SetInsertPoint(entry);
    builder.CreateRet(builder.getInt32(0));

    if (llvm::verifyModule(*module, &llvm::errs())) {
        std::cerr << "Error in module" << std::endl;
        return 1;
    }

    module->print(llvm::outs(), nullptr);

    return 0;
}