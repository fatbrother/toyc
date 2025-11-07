#pragma once

#include <string>
#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>

#include "ast/type.hpp"
#include "ast/node.hpp"

namespace toyc::utility {

toyc::ast::ExprCodegenResult castFromBool(llvm::Value *value, toyc::ast::VarType toType,
                                  toyc::ast::ASTContext &context);

toyc::ast::ExprCodegenResult castToBool(llvm::Value *value, toyc::ast::VarType fromType,
                                  toyc::ast::ASTContext &context);

toyc::ast::ExprCodegenResult typeCast(llvm::Value *value, const toyc::ast::NTypePtr fromType,
                                  const toyc::ast::NTypePtr toType, toyc::ast::ASTContext &context);

toyc::ast::ExprCodegenResult typeCast(llvm::Value *value, const toyc::ast::NTypePtr fromType,
                                  toyc::ast::VarType toType, toyc::ast::ASTContext &context);

} // namespace toyc::utility
