#include "semantic/parser_actions.hpp"

#include <iostream>

namespace toyc::semantic {

ParserActions::ParserActions(ast::TypeManager* typeManager) : typeManager_(typeManager), errorOccurred(false) {}

ParserActions::~ParserActions() {}

// External Declarations
ast::NExternalDeclaration* ParserActions::handleExternalDeclarationList(ast::NExternalDeclaration* current,
                                                                        ast::NExternalDeclaration* next) {
    if (current) {
        current->next = next;
    }
    return current;
}

// Function Definition
ast::NFunctionDefinition* ParserActions::handleFunctionDefinition(ast::TypeIdx returnTypeIdx, const std::string& name,
                                                                  ast::NParameter* params, ast::NBlock* body, int line,
                                                                  int column) {
    return new ast::NFunctionDefinition(returnTypeIdx, name, params, body);
}

ast::NFunctionDefinition* ParserActions::handleFunctionDeclaration(ast::TypeIdx returnTypeIdx, const std::string& name,
                                                                   ast::NParameter* params, int line, int column) {
    return new ast::NFunctionDefinition(returnTypeIdx, name, params, nullptr);
}

// Parameters
ast::NParameter* ParserActions::handleParameterList(ast::NParameter* current, ast::NParameter* next, int line,
                                                    int column) {
    if (current && current->isVariadic) {
        reportError("syntax error: variadic parameter must be the last parameter", line, column);
        return current;
    }

    if (current) {
        current->next = next;
    }
    return current;
}

ast::NParameter* ParserActions::handleParameter(ast::TypeIdx typeIdx, ast::NDeclarator* declarator, int line,
                                                int column) {
    // Array parameters are treated as pointers per C semantics
    ast::TypeIdx finalIdx = typeIdx;
    std::string name = "";
    if (nullptr != declarator) {
        name = declarator->getName();
        if (true == declarator->isArray()) {
            finalIdx = typeManager_->getPointerIdx(typeIdx, 1);
        } else if (0 < declarator->pointerLevel) {
            finalIdx = typeManager_->getPointerIdx(typeIdx, declarator->pointerLevel);
        }
    }
    return new ast::NParameter(finalIdx, std::move(name), declarator);
}

ast::NParameter* ParserActions::handleVariadicParameter() {
    return new ast::NParameter();  // Creates variadic parameter
}

// Statements
ast::NBlock* ParserActions::handleCompoundStatement(ast::NStatement* statements) {
    return new ast::NBlock(statements);
}

ast::NBlock* ParserActions::handleEmptyCompoundStatement() {
    return new ast::NBlock();
}

ast::NStatement* ParserActions::handleStatementList(ast::NStatement* current, ast::NStatement* next) {
    if (current) {
        current->next = next;
    }
    return current;
}

ast::NForStatement* ParserActions::handleForStatement(ast::NStatement* init, ast::NExpression* condition,
                                                      ast::NExpression* increment, ast::NStatement* body) {
    return new ast::NForStatement(init, condition, increment, body);
}

ast::NWhileStatement* ParserActions::handleWhileStatement(ast::NExpression* condition, ast::NStatement* body) {
    return new ast::NWhileStatement(condition, body);
}

ast::NWhileStatement* ParserActions::handleDoWhileStatement(ast::NExpression* condition, ast::NStatement* body) {
    return new ast::NWhileStatement(condition, body, true);
}

ast::NSwitchStatement* ParserActions::handleSwitchStatement(ast::NExpression* condition, ast::NBlock* body) {
    return new ast::NSwitchStatement(condition, body);
}

ast::NIfStatement* ParserActions::handleIfStatement(ast::NExpression* condition, ast::NStatement* thenBlock,
                                                    ast::NStatement* elseBlock) {
    return new ast::NIfStatement(condition, thenBlock, elseBlock);
}

// Labeled Statements
ast::NLabelStatement* ParserActions::handleLabelStatement(const std::string& label, ast::NStatement* statement) {
    return new ast::NLabelStatement(label, statement);
}

ast::NCaseStatement* ParserActions::handleCaseStatement(ast::NExpression* value) {
    return new ast::NCaseStatement(value);
}

ast::NCaseStatement* ParserActions::handleDefaultStatement() {
    return new ast::NCaseStatement(true);
}

// Jump Statements
ast::NGotoStatement* ParserActions::handleGotoStatement(const std::string& label) {
    return new ast::NGotoStatement(label);
}

ast::NReturnStatement* ParserActions::handleReturnStatement(ast::NExpression* expr) {
    return new ast::NReturnStatement(expr);
}

ast::NBreakStatement* ParserActions::handleBreakStatement() {
    return new ast::NBreakStatement();
}

ast::NContinueStatement* ParserActions::handleContinueStatement() {
    return new ast::NContinueStatement();
}

// Declarations
ast::NDeclarationStatement* ParserActions::handleDeclarationStatement(ast::TypeIdx typeIdx,
                                                                      ast::NDeclarator* declarator) {
    return new ast::NDeclarationStatement(typeIdx, declarator);
}

ast::NDeclarationStatement* ParserActions::handleEmptyDeclaration(ast::TypeIdx typeIdx) {
    return new ast::NDeclarationStatement(typeIdx, nullptr);
}

ast::NDeclarator* ParserActions::handleDeclaratorList(ast::NDeclarator* current, ast::NDeclarator* next) {
    if (current) {
        current->next = next;
    }
    return current;
}

ast::NDeclarator* ParserActions::handleInitDeclarator(ast::NDeclarator* declarator, ast::NExpression* initializer) {
    if (declarator && initializer) {
        declarator->expr = initializer;
    }
    return declarator;
}

ast::NDeclarator* ParserActions::handleDeclarator(int pointerLevel, ast::NDeclarator* declarator) {
    declarator->pointerLevel = pointerLevel;
    return declarator;
}

ast::NDeclarator* ParserActions::handleDeclarator(const std::string* name) {
    std::string declName = name ? *name : "";
    delete name;
    return new ast::NDeclarator(declName);
}

ast::NDeclarator* ParserActions::handleArrayDeclarator(ast::NDeclarator* declarator, ast::NExpression* arraySize) {
    if (nullptr == arraySize) {
        arraySize = handleInteger(0);
    }

    if (arraySize->getType() != "Integer") {
        declarator->isVLA = true;
    }
    declarator->addArrayDimension(arraySize);
    return declarator;
}

// Expressions
ast::NExpression* ParserActions::handleBinaryExpression(ast::BineryOperator op, ast::NExpression* left,
                                                        ast::NExpression* right) {
    // Check if it's a logical operator (AND, OR)
    if (op == ast::BineryOperator::AND || op == ast::BineryOperator::OR) {
        return new ast::NLogicalOperator(left, op, right);
    }
    return new ast::NBinaryOperator(left, op, right);
}

ast::NExpression* ParserActions::handleUnaryExpression(ast::UnaryOperator op, ast::NExpression* operand) {
    return new ast::NUnaryExpression(op, operand);
}

ast::NExpression* ParserActions::handleAssignment(ast::NExpression* left, ast::NExpression* right) {
    return new ast::NAssignment(left, right);
}

ast::NExpression* ParserActions::handleConditionalExpression(ast::NExpression* condition, ast::NExpression* trueExpr,
                                                             ast::NExpression* falseExpr) {
    return new ast::NConditionalExpression(condition, trueExpr, falseExpr);
}

ast::NExpression* ParserActions::handleFunctionCall(const std::string& name, ast::NArguments* args) {
    return new ast::NFunctionCall(name, args);
}

ast::NExpression* ParserActions::handleArrayAccess(ast::NExpression* array, ast::NExpression* index) {
    return new ast::NArraySubscript(array, index);
}

ast::NExpression* ParserActions::handleMemberAccess(ast::NExpression* object, const std::string& member,
                                                    bool isPointer) {
    return new ast::NMemberAccess(object, member, isPointer);
}

ast::NExpression* ParserActions::handleCastExpression(ast::TypeIdx typeIdx, ast::NExpression* expr) {
    return new ast::NCastExpression(typeIdx, expr);
}

ast::NExpression* ParserActions::handleCastExpressionWithPointer(ast::TypeIdx baseTypeIdx, int pointerLevel,
                                                                 ast::NExpression* expr) {
    ast::TypeIdx typeIdx = typeManager_->getPointerIdx(baseTypeIdx, pointerLevel);
    return new ast::NCastExpression(typeIdx, expr);
}

ast::NExpression* ParserActions::handleSizeofType(ast::TypeIdx typeIdx) {
    return new ast::NSizeofExpression(typeIdx);
}

ast::NExpression* ParserActions::handleSizeofExpression(ast::NExpression* expr) {
    return new ast::NSizeofExpression(expr);
}

// Primary Expressions
ast::NIdentifier* ParserActions::handleIdentifier(const std::string& name) {
    return new ast::NIdentifier(name);
}

ast::NInteger* ParserActions::handleInteger(int value) {
    return new ast::NInteger(value);
}

ast::NInteger* ParserActions::handleIntegerFromString(const std::string& value) {
    return new ast::NInteger(atoi(value.c_str()));
}

ast::NInteger* ParserActions::handleCharConstant(const std::string& value) {
    // Character constant like 'a' or '\n', extract the character
    // value format: 'c' or '\x'
    if (value.length() >= 3) {
        if (value[1] == '\\' && value.length() >= 4) {
            // Escape sequence
            switch (value[2]) {
                case 'n':
                    return new ast::NInteger('\n');
                case 't':
                    return new ast::NInteger('\t');
                case 'r':
                    return new ast::NInteger('\r');
                case '0':
                    return new ast::NInteger('\0');
                case '\\':
                    return new ast::NInteger('\\');
                case '\'':
                    return new ast::NInteger('\'');
                case '"':
                    return new ast::NInteger('"');
                case 'a':
                    return new ast::NInteger('\a');
                case 'b':
                    return new ast::NInteger('\b');
                case 'f':
                    return new ast::NInteger('\f');
                case 'v':
                    return new ast::NInteger('\v');
                case '?':
                    return new ast::NInteger('\?');
                default:
                    return new ast::NInteger(static_cast<int>(value[2]));
            }
        } else {
            // Regular character
            return new ast::NInteger(static_cast<int>(value[1]));
        }
    }
    return new ast::NInteger(0);
}

ast::NFloat* ParserActions::handleFloat(const std::string& value) {
    return new ast::NFloat(atof(value.c_str()));
}

ast::NString* ParserActions::handleString(const std::string& value) {
    return new ast::NString(value);
}

// Arguments
ast::NArguments* ParserActions::handleArgumentList(ast::NExpression* expr, ast::NArguments* next) {
    ast::NArguments* args = new ast::NArguments(expr);
    if (next) {
        args->next = next;
    }
    return args;
}

// Initializers
ast::NInitializerList* ParserActions::handleInitializerList(ast::NExpression* expr, ast::NInitializerList* next) {
    ast::NInitializerList* initList = new ast::NInitializerList();
    if (next) {
        // First add elements from existing list to maintain order
        for (auto* elem : next->getElements()) {
            initList->push_back(elem);
        }
    }
    // Then add the new element at the end
    initList->push_back(expr);
    return initList;
}

// Expression Statement
ast::NExpressionStatement* ParserActions::handleExpressionStatement(ast::NExpression* expr) {
    return new ast::NExpressionStatement(expr);
}

ast::NExpressionStatement* ParserActions::handleEmptyExpressionStatement() {
    return new ast::NExpressionStatement(nullptr);
}

// Comma Expression
ast::NExpression* ParserActions::handleCommaExpression(ast::NExpression* left, ast::NExpression* right) {
    return new ast::NCommaExpression(left, right);
}

// Compound Assignment
ast::NExpression* ParserActions::handleCompoundAssignment(ast::NExpression* left, ast::BineryOperator op,
                                                          ast::NExpression* right) {
    ast::NExpression* binaryOp = new ast::NBinaryOperator(left, op, right);
    return new ast::NAssignment(left, binaryOp);
}

// Type Specifiers
ast::TypeIdx ParserActions::handlePrimitiveType(const std::string& typeName) {
    if (typeName == "bool") {
        return typeManager_->getPrimitiveIdx(toyc::ast::VAR_TYPE_BOOL);
    } else if (typeName == "char") {
        return typeManager_->getPrimitiveIdx(toyc::ast::VAR_TYPE_CHAR);
    } else if (typeName == "short") {
        return typeManager_->getPrimitiveIdx(toyc::ast::VAR_TYPE_SHORT);
    } else if (typeName == "int") {
        return typeManager_->getPrimitiveIdx(toyc::ast::VAR_TYPE_INT);
    } else if (typeName == "long") {
        return typeManager_->getPrimitiveIdx(toyc::ast::VAR_TYPE_LONG);
    } else if (typeName == "float") {
        return typeManager_->getPrimitiveIdx(toyc::ast::VAR_TYPE_FLOAT);
    } else if (typeName == "double") {
        return typeManager_->getPrimitiveIdx(toyc::ast::VAR_TYPE_DOUBLE);
    } else if (typeName == "void") {
        return typeManager_->getPrimitiveIdx(toyc::ast::VAR_TYPE_VOID);
    }
    return ast::InvalidTypeIdx;
}

ast::TypeIdx ParserActions::handlePointerType(ast::TypeIdx baseTypeIdx, int pointerLevel) {
    if (pointerLevel <= 0) {
        return baseTypeIdx;
    }
    return typeManager_->getPointerIdx(baseTypeIdx, pointerLevel);
}

// Struct
ast::NStructDeclaration* ParserActions::handleStructDeclaration(ast::TypeIdx typeIdx, ast::NDeclarator* declarator) {
    return new ast::NStructDeclaration(typeIdx, declarator);
}

ast::NStructDeclaration* ParserActions::handleStructDeclarationList(ast::NStructDeclaration* current,
                                                                    ast::NStructDeclaration* next) {
    if (current) {
        current->next = next;
    }
    return current;
}

ast::TypeIdx ParserActions::handleStructSpecifier(const std::string& name, ast::NStructDeclaration* declarations) {
    return typeManager_->getStructIdx(name, declarations);
}

ast::TypeIdx ParserActions::handleAnonymousStruct(ast::NStructDeclaration* declarations) {
    return typeManager_->getStructIdx("", declarations);
}

ast::TypeIdx ParserActions::handleStructReference(const std::string& name) {
    return typeManager_->getStructIdx(name, nullptr);
}

ast::TypeIdx ParserActions::handleTypeNameWithPointer(ast::TypeIdx baseTypeIdx, int pointerLevel) {
    return typeManager_->getPointerIdx(baseTypeIdx, pointerLevel);
}

ast::TypeIdx ParserActions::handleTypeNameWithArray(ast::TypeIdx baseTypeIdx, const std::string& arraySize) {
    std::vector<int> dims = {std::stoi(arraySize)};
    return typeManager_->getArrayIdx(baseTypeIdx, std::move(dims));
}

ast::TypeIdx ParserActions::handleTypeNameWithPointerAndArray(ast::TypeIdx baseTypeIdx, int pointerLevel,
                                                              const std::string& arraySize) {
    ast::TypeIdx ptrIdx = typeManager_->getPointerIdx(baseTypeIdx, pointerLevel);
    std::vector<int> dims = {std::stoi(arraySize)};
    return typeManager_->getArrayIdx(ptrIdx, std::move(dims));
}

// Error reporting
void ParserActions::reportError(const std::string& message, int line, int column) {
    std::cerr << "Parser error";
    if (line > 0) {
        std::cerr << " at line " << line;
        if (column > 0) {
            std::cerr << ", column " << column;
        }
    }
    std::cerr << ": " << message << std::endl;
    errorOccurred = true;
}

}  // namespace toyc::semantic
