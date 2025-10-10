#pragma once

#include "ast/expression.hpp"

namespace toyc::ast {

class NStructDeclaration : public BasicNode {
public:
    NStructDeclaration(NType *type, NDeclarator *declarator)
        : type(type), declarator(declarator) {}
    ~NStructDeclaration() {
        SAFE_DELETE(declarator);
    }
    virtual std::string getType() const override { return "StructDeclaration"; }

    NTypePtr type;
    NDeclarator *declarator = nullptr;
    NStructDeclaration *next = nullptr;
};

class NStructType : public NType {
public:
    NStructType(const std::string &name, NStructDeclaration *members)
        : NType(VarType::VAR_TYPE_STRUCT), name(name), members(members) {}
    ~NStructType() {
        SAFE_DELETE(members);
    }
    virtual std::string getType() const override { return "StructType"; }
    const std::string &getName() const { return name; }

private:
    std::string name;
    NStructDeclaration *members = nullptr;
};

} // namespace toyc::ast