#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "ast/expression.hpp"
#include "ast/external_definition.hpp"
#include "ast/node.hpp"
#include "ast/statement.hpp"
#include "ast/type.hpp"

namespace toyc::semantic {

/**
 * @brief Parser Actions - handles all grammar rule actions
 *
 * This class centralizes all parser actions from the grammar file,
 * providing better maintainability and enabling semantic processing
 * during parsing (e.g., symbol table management, type checking).
 */
class ParserActions {
public:
    explicit ParserActions(ast::TypeManager* typeManager);
    ~ParserActions();

    // External Declarations
    ast::NExternalDeclaration* handleExternalDeclarationList(ast::NExternalDeclaration* current,
                                                             ast::NExternalDeclaration* next);

    // Function Definition
    ast::NFunctionDefinition* handleFunctionDefinition(ast::TypeIdx returnTypeIdx, const std::string& name,
                                                       ast::NParameter* params, ast::NBlock* body, int line = 0,
                                                       int column = 0);

    ast::NFunctionDefinition* handleFunctionDeclaration(ast::TypeIdx returnTypeIdx, const std::string& name,
                                                        ast::NParameter* params, int line = 0, int column = 0);

    // Parameters
    ast::NParameter* handleParameterList(ast::NParameter* current, ast::NParameter* next, int line = 0, int column = 0);

    ast::NParameter* handleParameter(ast::TypeIdx typeIdx, ast::NDeclarator* declarator, int line = 0, int column = 0);

    ast::NParameter* handleVariadicParameter();

    // Statements
    ast::NBlock* handleCompoundStatement(ast::NStatement* statements);
    ast::NBlock* handleEmptyCompoundStatement();

    ast::NStatement* handleStatementList(ast::NStatement* current, ast::NStatement* next);

    ast::NForStatement* handleForStatement(ast::NStatement* init, ast::NExpression* condition,
                                           ast::NExpression* increment, ast::NStatement* body);

    ast::NWhileStatement* handleWhileStatement(ast::NExpression* condition, ast::NStatement* body);

    ast::NWhileStatement* handleDoWhileStatement(ast::NExpression* condition, ast::NStatement* body);

    ast::NSwitchStatement* handleSwitchStatement(ast::NExpression* condition, ast::NBlock* body);

    ast::NIfStatement* handleIfStatement(ast::NExpression* condition, ast::NStatement* thenBlock,
                                         ast::NStatement* elseBlock = nullptr);

    // Labeled Statements
    ast::NLabelStatement* handleLabelStatement(const std::string& label, ast::NStatement* statement);

    ast::NCaseStatement* handleCaseStatement(ast::NExpression* value);
    ast::NCaseStatement* handleDefaultStatement();

    // Jump Statements
    ast::NGotoStatement* handleGotoStatement(const std::string& label);
    ast::NReturnStatement* handleReturnStatement(ast::NExpression* expr = nullptr);
    ast::NBreakStatement* handleBreakStatement();
    ast::NContinueStatement* handleContinueStatement();

    // Declarations
    ast::NDeclarationStatement* handleDeclarationStatement(ast::TypeIdx typeIdx, ast::NDeclarator* declarator);

    ast::NDeclarationStatement* handleEmptyDeclaration(ast::TypeIdx typeIdx);

    ast::NDeclarator* handleDeclaratorList(ast::NDeclarator* current, ast::NDeclarator* next);

    ast::NDeclarator* handleInitDeclarator(ast::NDeclarator* declarator, ast::NExpression* initializer = nullptr);

    ast::NDeclarator* handleDeclarator(int pointerLevel, ast::NDeclarator* declarator);

    ast::NDeclarator* handleDeclarator(const std::string* name);

    ast::NDeclarator* handleArrayDeclarator(ast::NDeclarator* declarator, ast::NExpression* arraySize = nullptr);

    // Expressions
    ast::NExpression* handleBinaryExpression(ast::BineryOperator op, ast::NExpression* left, ast::NExpression* right);

    ast::NExpression* handleUnaryExpression(ast::UnaryOperator op, ast::NExpression* operand);

    ast::NExpression* handleAssignment(ast::NExpression* left, ast::NExpression* right);

    ast::NExpression* handleConditionalExpression(ast::NExpression* condition, ast::NExpression* trueExpr,
                                                  ast::NExpression* falseExpr);

    ast::NExpression* handleFunctionCall(const std::string& name, ast::NArguments* args = nullptr);

    ast::NExpression* handleArrayAccess(ast::NExpression* array, ast::NExpression* index);

    ast::NExpression* handleMemberAccess(ast::NExpression* object, const std::string& member, bool isPointer = false);

    ast::NExpression* handleCastExpression(ast::TypeIdx typeIdx, ast::NExpression* expr);

    ast::NExpression* handleCastExpressionWithPointer(ast::TypeIdx baseTypeIdx, int pointerLevel,
                                                      ast::NExpression* expr);

    ast::NExpression* handleSizeofType(ast::TypeIdx typeIdx);
    ast::NExpression* handleSizeofExpression(ast::NExpression* expr);

    // Primary Expressions
    ast::NIdentifier* handleIdentifier(const std::string& name);
    ast::NInteger* handleInteger(int value);
    ast::NInteger* handleIntegerFromString(const std::string& value);
    ast::NInteger* handleCharConstant(const std::string& value);
    ast::NFloat* handleFloat(const std::string& value);
    ast::NString* handleString(const std::string& value);

    // Arguments
    ast::NArguments* handleArgumentList(ast::NExpression* expr, ast::NArguments* next = nullptr);

    // Initializers
    ast::NInitializerList* handleInitializerList(ast::NExpression* expr, ast::NInitializerList* next = nullptr);

    // Expression Statement
    ast::NExpressionStatement* handleExpressionStatement(ast::NExpression* expr);
    ast::NExpressionStatement* handleEmptyExpressionStatement();

    // Comma Expression
    ast::NExpression* handleCommaExpression(ast::NExpression* left, ast::NExpression* right);

    // Compound Assignment
    ast::NExpression* handleCompoundAssignment(ast::NExpression* left, ast::BineryOperator op, ast::NExpression* right);

    // Type Specifiers — return TypeIdx registered in TypeManager
    ast::TypeIdx handlePrimitiveType(const std::string& typeName);
    ast::TypeIdx handlePointerType(ast::TypeIdx baseTypeIdx, int pointerLevel);
    ast::TypeIdx handleQualifiedType(ast::TypeIdx baseTypeIdx, uint8_t qualifiers);

    // Struct
    ast::NStructDeclaration* handleStructDeclaration(ast::TypeIdx typeIdx, ast::NDeclarator* declarator);

    ast::NStructDeclaration* handleStructDeclarationList(ast::NStructDeclaration* current,
                                                         ast::NStructDeclaration* next);

    ast::TypeIdx handleStructSpecifier(const std::string& name, ast::NStructDeclaration* declarations = nullptr);

    ast::TypeIdx handleAnonymousStruct(ast::NStructDeclaration* declarations);
    ast::TypeIdx handleStructReference(const std::string& name);

    // Type name with pointer/array — return TypeIdx
    ast::TypeIdx handleTypeNameWithPointer(ast::TypeIdx baseTypeIdx, int pointerLevel);

    ast::TypeIdx handleTypeNameWithArray(ast::TypeIdx baseTypeIdx, const std::string& arraySize);

    ast::TypeIdx handleTypeNameWithPointerAndArray(ast::TypeIdx baseTypeIdx, int pointerLevel,
                                                   const std::string& arraySize);

    // Error reporting
    void reportError(const std::string& message, int line = 0, int column = 0);
    bool hasError() const { return errorOccurred; }
    void clearError() { errorOccurred = false; }

private:
    ast::TypeManager* typeManager_;
    bool errorOccurred;
};

}  // namespace toyc::semantic
