#pragma once

#include <iostream>
#include <memory>

#include "ast/node.hpp"

namespace toyc::ast {

class NExpression : public BasicNode {
public:
    virtual ExprCodegenResult codegen(ASTContext &context) = 0;
    virtual AllocCodegenResult allocgen(ASTContext & /*context*/) {
        return AllocCodegenResult("Allocation not supported for " + getType());
    }
    virtual std::string getType() const override { return "Expression"; }
};

class NBinaryOperator : public NExpression {
public:
    NBinaryOperator(NExpression *lhs, BineryOperator op, NExpression *rhs) : lhs(lhs), rhs(rhs), op(op) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "BinaryOperator"; }

protected:
    std::unique_ptr<NExpression> lhs;
    std::unique_ptr<NExpression> rhs;
    BineryOperator op;
};

class NLogicalOperator : public NBinaryOperator {
public:
    NLogicalOperator(NExpression *lhs, BineryOperator op, NExpression *rhs) : NBinaryOperator(lhs, op, rhs) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "LogicalOperator"; }
};

class NUnaryExpression : public NExpression {
public:
    NUnaryExpression(UnaryOperator op, NExpression *expr) : op(op), expr(expr) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "UnaryOperator"; }
    virtual AllocCodegenResult allocgen(ASTContext &context) override { return expr->allocgen(context); };

private:
    UnaryOperator op;
    std::unique_ptr<NExpression> expr;
};

class NConditionalExpression : public NExpression {
public:
    NConditionalExpression(NExpression *condition, NExpression *trueExpr, NExpression *falseExpr)
        : condition(condition), trueExpr(trueExpr), falseExpr(falseExpr) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "ConditionalExpression"; }

private:
    std::unique_ptr<NExpression> condition;
    std::unique_ptr<NExpression> trueExpr;
    std::unique_ptr<NExpression> falseExpr;
};

class NIdentifier : public NExpression {
public:
    explicit NIdentifier(const std::string &name) : name(name) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual AllocCodegenResult allocgen(ASTContext &context) override;
    virtual std::string getType() const override { return "Identifier"; }

private:
    std::string name;
};

class NInteger : public NExpression {
public:
    explicit NInteger(int value) : value(value) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "Integer"; }
    int getValue() const { return value; }

private:
    int value;
};

class NFloat : public NExpression {
public:
    explicit NFloat(double value) : value(value) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "Float"; }

private:
    double value;
};

class NString : public NExpression {
public:
    explicit NString(const std::string &value) : value(value) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "String"; }

private:
    std::string value;
};

class NDeclarator : public NExpression {
public:
    explicit NDeclarator(const std::string &name, int pointerLevel = 0) : pointerLevel(pointerLevel), name(name) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override {
        if (nullptr == expr) {
            return ExprCodegenResult();
        }
        return expr->codegen(context);
    }
    virtual std::string getType() const override { return "Declaration"; }
    std::string getName() const { return name; }
    bool isNonInitialized() const { return expr == nullptr; }
    CodegenResult<llvm::Value *> getArraySizeValue(ASTContext &context);

    void addArrayDimension(NExpression *size) { arrayDimensions.emplace_back(size); }

    int getArrayDimensionCount() const { return static_cast<int>(arrayDimensions.size()); }

    bool isArray() const { return !arrayDimensions.empty(); }

    bool isPointer() const { return pointerLevel > 0; }

    const std::vector<std::unique_ptr<NExpression>> &getArrayDimensions() const { return arrayDimensions; }

public:
    int pointerLevel = 0;
    uint8_t qualifiers = QUAL_NONE;
    std::unique_ptr<NDeclarator> next;
    std::unique_ptr<NExpression> expr;
    std::vector<std::unique_ptr<NExpression>> arrayDimensions;
    bool isVLA = false;

private:
    std::string name;
};

class NAssignment : public NExpression {
public:
    NAssignment(NExpression *lhs, NExpression *rhs) : lhs(lhs), rhs(rhs) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "Assignment"; }

private:
    std::unique_ptr<NExpression> lhs;
    std::unique_ptr<NExpression> rhs;
};

class NArguments : public NExpression {
public:
    explicit NArguments(NExpression *expr) : expr(expr) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "Arguments"; }

public:
    std::unique_ptr<NArguments> next;
    std::unique_ptr<NExpression> expr;
};

class NFunctionCall : public NExpression {
public:
    NFunctionCall(const std::string &name, NArguments *argNodes) : name(name), argNodes(argNodes) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "FunctionCall"; }

private:
    std::string name;
    std::unique_ptr<NArguments> argNodes;
};

class NMemberAccess : public NExpression {
public:
    NMemberAccess(NExpression *base, const std::string &memberName, bool isPointerAccess)
        : base(base), memberName(memberName), isPointerAccess(isPointerAccess) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual AllocCodegenResult allocgen(ASTContext &context) override;
    virtual std::string getType() const override { return "MemberAccess"; }

private:
    std::unique_ptr<NExpression> base;
    std::string memberName;
    bool isPointerAccess;
};

class NArraySubscript : public NExpression {
public:
    NArraySubscript(NExpression *array, NExpression *index) : array(array), index(index) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual AllocCodegenResult allocgen(ASTContext &context) override;
    virtual std::string getType() const override { return "ArraySubscript"; }

private:
    std::unique_ptr<NExpression> array;
    std::unique_ptr<NExpression> index;
};

class NInitializerList : public NExpression {
public:
    NInitializerList() {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "InitializerList"; }

    const std::vector<std::unique_ptr<NExpression>> &getElements() const { return elements; }
    void push_back(NExpression *expr) { elements.emplace_back(expr); }
    void push_back(std::unique_ptr<NExpression> expr) { elements.push_back(std::move(expr)); }

private:
    std::vector<std::unique_ptr<NExpression>> elements;
};

// Cast expression: (type) expression
class NCastExpression : public NExpression {
public:
    NCastExpression(TypeIdx targetTypeIdx, NExpression *expr) : targetTypeIdx(targetTypeIdx), expr(expr) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "CastExpression"; }

private:
    TypeIdx targetTypeIdx;
    std::unique_ptr<NExpression> expr;
};

// Sizeof operator: sizeof(type) or sizeof expression
class NSizeofExpression : public NExpression {
public:
    // sizeof(type)
    explicit NSizeofExpression(TypeIdx targetTypeIdx) : targetTypeIdx(targetTypeIdx), isSizeofType(true) {}
    // sizeof expression
    explicit NSizeofExpression(NExpression *expr) : targetTypeIdx(InvalidTypeIdx), expr(expr), isSizeofType(false) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "SizeofExpression"; }

private:
    TypeIdx targetTypeIdx;
    std::unique_ptr<NExpression> expr;
    bool isSizeofType;
};

class NCompoundAssignment : public NExpression {
public:
    NCompoundAssignment(NExpression *lhs, BineryOperator op, NExpression *rhs) : lhs(lhs), op(op), rhs(rhs) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "CompoundAssignment"; }

private:
    std::unique_ptr<NExpression> lhs;
    BineryOperator op;
    std::unique_ptr<NExpression> rhs;
};

// Comma operator: expr1, expr2
class NCommaExpression : public NExpression {
public:
    NCommaExpression(NExpression *left, NExpression *right) : left(left), right(right) {}
    virtual ExprCodegenResult codegen(ASTContext &context) override;
    virtual std::string getType() const override { return "CommaExpression"; }

private:
    std::unique_ptr<NExpression> left;
    std::unique_ptr<NExpression> right;
};

}  // namespace toyc::ast
