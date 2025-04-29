#pragma once

#include <string>
#include <map>
#include <iostream>
#include <llvm/IR/Value.h>
#include <llvm/IR/IRBuilder.h>

namespace toyc::ast {

class BasicNode {
public:
    virtual ~BasicNode() = default;
    BasicNode() = default;
    BasicNode(const BasicNode &) = default;
    BasicNode(BasicNode &&) = default;
    BasicNode &operator=(const BasicNode &) = default;
    virtual std::string getType() const = 0;
};

class NType : public BasicNode {
public:
    NType(int type, const std::string &name = "")
        : type(type), name(name) {}
    virtual std::string getType() const override { return "Type"; }
    virtual llvm::Type *getLLVMType(llvm::LLVMContext &context) const;

private:
    int type;
    std::string name;
};

class NBlock;

} // namespace toyc::ast
