#include "ast/node.hpp"
#include "ast/expression.hpp"
#include "ast/statement.hpp"
#include "ast/external_definition.hpp"
#include "parser/y.tab.hpp"

#include <iostream>

using namespace toyc::ast;

llvm::Type *NType::getLLVMType(llvm::LLVMContext &context) const {
    llvm::Type *llvmType = nullptr;

    if (false == name.empty()) {
        // TODO: Handle named types
        goto FUNC_OUT;
    } else {
        switch (type) {
            case INT:
                llvmType = llvm::Type::getInt32Ty(context);
                break;
            case FLOAT:
                llvmType = llvm::Type::getFloatTy(context);
                break;
            case DOUBLE:
                llvmType = llvm::Type::getDoubleTy(context);
                break;
            case VOID:
                llvmType = llvm::Type::getVoidTy(context);
                break;
            default:
                std::cerr << "Unknown type: " << type << std::endl;
                break;
        }
    }

FUNC_OUT:

    return llvmType;
}