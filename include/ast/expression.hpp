#pragma once

#include "ast/node.hpp"

#include <iostream>

namespace toyc::ast {

class NExpression : public BasicNode {
public:
    virtual ExprCodegenResult codegen(ASTContext &context) = 0;
    virtual AllocCodegenResult allocgen(ASTContext &context) {
        return AllocCodegenResult("Allocation not supported for " + getType());
    }
    virtual std::string getType() const override { return "Expression"; }
};

class NBinaryOperator : public NExpression {
public:
    NBinaryOperator(NExpression *lhs, BineryOperator op, NExpression *rhs)
        : lhs(lhs), rhs(rhs), op(op) {}
    ~NBinaryOperator() {
        SAFE_DELETE(lhs);
        SAFE_DELETE(rhs);
    }
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "BinaryOperator"; }

protected:
    NExpression *lhs;
    NExpression *rhs;
    BineryOperator op;
};

class NLogicalOperator : public NBinaryOperator {
public:
    NLogicalOperator(NExpression *lhs, BineryOperator op, NExpression *rhs)
        : NBinaryOperator(lhs, op, rhs) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "LogicalOperator"; }
};

class NUnaryExpression : public NExpression {
public:
    NUnaryExpression(UnaryOperator op, NExpression *expr)
        : op(op), expr(expr) {}
    ~NUnaryExpression() {
        SAFE_DELETE(expr);
    }
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "UnaryOperator"; }
    virtual AllocCodegenResult allocgen(ASTContext &context) override {
        return expr->allocgen(context);
    };

private:
    UnaryOperator op;
    NExpression *expr;
};

class NConditionalExpression : public NExpression {
public:
    NConditionalExpression(NExpression *condition, NExpression *trueExpr, NExpression *falseExpr)
        : condition(condition), trueExpr(trueExpr), falseExpr(falseExpr) {}
    ~NConditionalExpression() {
        SAFE_DELETE(condition);
        SAFE_DELETE(trueExpr);
        SAFE_DELETE(falseExpr);
    }
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "ConditionalExpression"; }

private:
    NExpression *condition;
    NExpression *trueExpr;
    NExpression *falseExpr;
};

class NIdentifier : public NExpression {
public:
    NIdentifier(const std::string &name) : name(name) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual AllocCodegenResult allocgen(ASTContext &context) override;
    virtual std::string getType() const override { return "Identifier"; }

private:
    std::string name;
};

class NInteger : public NExpression {
public:
    NInteger(int value) : value(value) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "Integer"; }

private:
    int value;
};

class NFloat : public NExpression {
public:
    NFloat(double value) : value(value) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "Float"; }

private:
    double value;
};

class NString : public NExpression {
public:
    NString(const std::string &value) : value(value) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "String"; }

private:
    std::string value;
};

class NDeclarator : public NExpression {
public:
    NDeclarator(const std::string &name, int pointerLevel = 0)
        : name(name), pointerLevel(pointerLevel) {}
    ~NDeclarator() {
        SAFE_DELETE(expr);
        SAFE_DELETE(next);
    }
    virtual ExprCodegenResult codegen(ASTContext &context) override {
        if (nullptr == expr) {
            return ExprCodegenResult();
        }
        return expr->codegen(context);
    }
    virtual std::string getType() const override { return "Declaration"; }
    std::string getName() const { return name; }
    bool isNonInitialized() const { return expr == nullptr; }

    void addArrayDimension(int size) {
        arrayDimensions.push_back(size);
    }

    bool isArray() const {
        return !arrayDimensions.empty();
    }

    bool isPointer() const {
        return pointerLevel > 0;
    }

    const std::vector<int>& getArrayDimensions() const {
        return arrayDimensions;
    }

public:
    int pointerLevel = 0;
    NDeclarator *next = nullptr;
    NExpression *expr = nullptr;
    std::vector<int> arrayDimensions;

private:
    std::string name;
};

class NAssignment : public NExpression {
public:
    NAssignment(NExpression *lhs, NExpression *rhs)
        : lhs(lhs), rhs(rhs) {}
    ~NAssignment() {
        SAFE_DELETE(lhs);
        SAFE_DELETE(rhs);
    }
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "Assignment"; }

private:
    NExpression *lhs;
    NExpression *rhs;
};

class NArguments : public NExpression {
public:
    NArguments(NExpression *expr) : expr(expr) {}
    ~NArguments() {
        SAFE_DELETE(expr);
        SAFE_DELETE(next);
    }
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "Arguments"; }

public:
    NArguments *next = nullptr;
    NExpression *expr = nullptr;
};

class NFunctionCall : public NExpression {
public:
    NFunctionCall(const std::string &name, NArguments *argNodes)
        : name(name), argNodes(argNodes) {}
    ~NFunctionCall() {
        SAFE_DELETE(argNodes);
    }
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "FunctionCall"; }

private:
    std::string name;
    NArguments *argNodes;
};

class NMemberAccess : public NExpression {
public:
    NMemberAccess(NExpression *base, const std::string &memberName, bool isPointerAccess)
        : base(base), memberName(memberName), isPointerAccess(isPointerAccess) {}
    ~NMemberAccess() {
        SAFE_DELETE(base);
    }
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual AllocCodegenResult allocgen(ASTContext &context) override;
    virtual std::string getType() const override { return "MemberAccess"; }

private:
    NExpression *base;
    std::string memberName;
    bool isPointerAccess;
};

class NArraySubscript : public NExpression {
public:
    NArraySubscript(NExpression *array, NExpression *index)
        : array(array), index(index) {}
    ~NArraySubscript() {
        SAFE_DELETE(array);
        SAFE_DELETE(index);
    }
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual AllocCodegenResult allocgen(ASTContext &context) override;
    virtual std::string getType() const override { return "ArraySubscript"; }

private:
    NExpression *array;
    NExpression *index;
};

class NInitializerList : public NExpression {
public:
    NInitializerList() {}
    ~NInitializerList() {
        for (auto elem : elements) {
            SAFE_DELETE(elem);
        }
    }
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "InitializerList"; }

    const std::vector<NExpression*>& getElements() const { return elements; }
    void push_back(NExpression *expr) { elements.push_back(expr); }

private:
    std::vector<NExpression*> elements;
};

// Cast expression: (type) expression
class NCastExpression : public NExpression {
public:
    NCastExpression(TypeDescriptor *targetTypeDesc, NExpression *expr)
        : targetTypeDesc(targetTypeDesc), expr(expr) {}
    ~NCastExpression() {
        SAFE_DELETE(targetTypeDesc);
        SAFE_DELETE(expr);
    }
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "CastExpression"; }

private:
    TypeDescriptor *targetTypeDesc;
    NExpression *expr;
};

// Sizeof operator: sizeof(type) or sizeof expression
class NSizeofExpression : public NExpression {
public:
    // sizeof(type)
    NSizeofExpression(TypeDescriptor *targetTypeDesc)
        : targetTypeDesc(targetTypeDesc), expr(nullptr), isSizeofType(true) {}
    // sizeof expression
    NSizeofExpression(NExpression *expr)
        : targetTypeDesc(nullptr), expr(expr), isSizeofType(false) {}
    ~NSizeofExpression() {
        SAFE_DELETE(targetTypeDesc);
        SAFE_DELETE(expr);
    }
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "SizeofExpression"; }

private:
    TypeDescriptor *targetTypeDesc;
    NExpression *expr;
    bool isSizeofType;
};

// Comma operator: expr1, expr2
class NCommaExpression : public NExpression {
public:
    NCommaExpression(NExpression *left, NExpression *right)
        : left(left), right(right) {}
    ~NCommaExpression() {
        SAFE_DELETE(left);
        SAFE_DELETE(right);
    }
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "CommaExpression"; }

private:
    NExpression *left;
    NExpression *right;
};

} // namespace toyc::ast