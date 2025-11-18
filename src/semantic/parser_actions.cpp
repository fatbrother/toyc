#include "semantic/parser_actions.hpp"
#include <iostream>

namespace toyc::semantic {

ParserActions::ParserActions()
    : errorOccurred(false) {
}

ParserActions::~ParserActions() {
    // Clean up cached identifiers
    for (auto& pair : identifierCache) {
        delete pair.second;
    }
    identifierCache.clear();
}

// Helper: Get or create identifier (enables node reuse)
ast::NIdentifier* ParserActions::getOrCreateIdentifier(const std::string& name) {
    auto it = identifierCache.find(name);
    if (it != identifierCache.end()) {
        return it->second;
    }
    
    ast::NIdentifier* identifier = new ast::NIdentifier(name);
    identifierCache[name] = identifier;
    return identifier;
}

// External Declarations
ast::NExternalDeclaration* ParserActions::handleExternalDeclarationList(
    ast::NExternalDeclaration* current,
    ast::NExternalDeclaration* next
) {
    if (current) {
        current->next = next;
    }
    return current;
}

// Function Definition
ast::NFunctionDefinition* ParserActions::handleFunctionDefinition(
    ast::TypeDescriptor* returnType,
    const std::string& name,
    ast::NParameter* params,
    ast::NBlock* body,
    int line,
    int column
) {
    // Semantic check: function should have a body or be a forward declaration
    return new ast::NFunctionDefinition(returnType, name, params, body);
}

ast::NFunctionDefinition* ParserActions::handleFunctionDeclaration(
    ast::TypeDescriptor* returnType,
    const std::string& name,
    ast::NParameter* params,
    int line,
    int column
) {
    // Forward declaration (no body)
    return new ast::NFunctionDefinition(returnType, name, params, nullptr);
}

// Parameters
ast::NParameter* ParserActions::handleParameterList(
    ast::NParameter* current,
    ast::NParameter* next,
    int line,
    int column
) {
    if (current && current->isVariadic) {
        reportError("syntax error: variadic parameter must be the last parameter", line, column);
        return current;
    }
    
    if (current) {
        current->next = next;
    }
    return current;
}

ast::NParameter* ParserActions::handleParameter(
    ast::TypeDescriptor* type,
    ast::NDeclarator* declarator,
    int line,
    int column
) {
    return new ast::NParameter(type, declarator);
}

ast::NParameter* ParserActions::handleVariadicParameter() {
    return new ast::NParameter(); // Creates variadic parameter
}

// Statements
ast::NBlock* ParserActions::handleCompoundStatement(ast::NStatement* statements) {
    return new ast::NBlock(statements);
}

ast::NBlock* ParserActions::handleEmptyCompoundStatement() {
    return new ast::NBlock();
}

ast::NStatement* ParserActions::handleStatementList(
    ast::NStatement* current,
    ast::NStatement* next
) {
    if (current) {
        current->next = next;
    }
    return current;
}

ast::NForStatement* ParserActions::handleForStatement(
    ast::NStatement* init,
    ast::NExpression* condition,
    ast::NExpression* increment,
    ast::NStatement* body
) {
    return new ast::NForStatement(init, condition, increment, body);
}

ast::NWhileStatement* ParserActions::handleWhileStatement(
    ast::NExpression* condition,
    ast::NStatement* body
) {
    return new ast::NWhileStatement(condition, body);
}

ast::NWhileStatement* ParserActions::handleDoWhileStatement(
    ast::NExpression* condition,
    ast::NStatement* body
) {
    return new ast::NWhileStatement(condition, body, true);
}

ast::NSwitchStatement* ParserActions::handleSwitchStatement(
    ast::NExpression* condition,
    ast::NBlock* body
) {
    return new ast::NSwitchStatement(condition, body);
}

ast::NIfStatement* ParserActions::handleIfStatement(
    ast::NExpression* condition,
    ast::NStatement* thenBlock,
    ast::NStatement* elseBlock
) {
    return new ast::NIfStatement(condition, thenBlock, elseBlock);
}

// Labeled Statements
ast::NLabelStatement* ParserActions::handleLabelStatement(
    const std::string& label,
    ast::NStatement* statement
) {
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
ast::NDeclarationStatement* ParserActions::handleDeclarationStatement(
    ast::TypeDescriptor* type,
    ast::NDeclarator* declarator
) {
    return new ast::NDeclarationStatement(type, declarator);
}

ast::NDeclarationStatement* ParserActions::handleEmptyDeclaration(ast::TypeDescriptor* type) {
    return new ast::NDeclarationStatement(type, nullptr);
}

ast::NDeclarator* ParserActions::handleDeclaratorList(
    ast::NDeclarator* current,
    ast::NDeclarator* next
) {
    if (current) {
        current->next = next;
    }
    return current;
}

ast::NDeclarator* ParserActions::handleInitDeclarator(
    ast::NDeclarator* declarator,
    ast::NExpression* initializer
) {
    if (declarator && initializer) {
        declarator->expr = initializer;
    }
    return declarator;
}

ast::NDeclarator* ParserActions::handleDeclarator(
    int pointerLevel,
    const std::string& name
) {
    return new ast::NDeclarator(name, pointerLevel);
}

ast::NDeclarator* ParserActions::handleArrayDeclarator(
    int pointerLevel,
    const std::string& name,
    int arraySize
) {
    ast::NDeclarator* decl = new ast::NDeclarator(name, pointerLevel);
    decl->addArrayDimension(arraySize);
    return decl;
}

ast::NDeclarator* ParserActions::handleMultiDimArrayDeclarator(
    int pointerLevel,
    const std::string& name,
    ast::NExpression* dimensions
) {
    ast::NDeclarator* decl = new ast::NDeclarator(name, pointerLevel);
    // Note: Would need to parse dimensions expression for multi-dimensional arrays
    // For now, just create a basic declarator
    return decl;
}

// Expressions
ast::NExpression* ParserActions::handleBinaryExpression(
    ast::BineryOperator op,
    ast::NExpression* left,
    ast::NExpression* right
) {
    // Check if it's a logical operator (AND, OR)
    if (op == ast::BineryOperator::AND || op == ast::BineryOperator::OR) {
        return new ast::NLogicalOperator(left, op, right);
    }
    return new ast::NBinaryOperator(left, op, right);
}

ast::NExpression* ParserActions::handleUnaryExpression(
    ast::UnaryOperator op,
    ast::NExpression* operand
) {
    return new ast::NUnaryExpression(op, operand);
}

ast::NExpression* ParserActions::handleAssignment(
    ast::NExpression* left,
    ast::NExpression* right
) {
    return new ast::NAssignment(left, right);
}

ast::NExpression* ParserActions::handleConditionalExpression(
    ast::NExpression* condition,
    ast::NExpression* trueExpr,
    ast::NExpression* falseExpr
) {
    return new ast::NConditionalExpression(condition, trueExpr, falseExpr);
}

ast::NExpression* ParserActions::handleFunctionCall(
    const std::string& name,
    ast::NArguments* args
) {
    return new ast::NFunctionCall(name, args);
}

ast::NExpression* ParserActions::handleArrayAccess(
    ast::NExpression* array,
    ast::NExpression* index
) {
    return new ast::NArraySubscript(array, index);
}

ast::NExpression* ParserActions::handleMemberAccess(
    ast::NExpression* object,
    const std::string& member,
    bool isPointer
) {
    return new ast::NMemberAccess(object, member, isPointer);
}

ast::NExpression* ParserActions::handleCastExpression(
    ast::TypeDescriptor* type,
    ast::NExpression* expr
) {
    return new ast::NCastExpression(type, expr);
}

ast::NExpression* ParserActions::handleSizeofType(ast::TypeDescriptor* type) {
    return new ast::NSizeofExpression(type);
}

ast::NExpression* ParserActions::handleSizeofExpression(ast::NExpression* expr) {
    return new ast::NSizeofExpression(expr);
}

// Primary Expressions
ast::NIdentifier* ParserActions::handleIdentifier(const std::string& name) {
    return getOrCreateIdentifier(name);
}

ast::NInteger* ParserActions::handleInteger(int value) {
    return new ast::NInteger(value);
}

ast::NString* ParserActions::handleString(const std::string& value) {
    return new ast::NString(value);
}

// Arguments
ast::NArguments* ParserActions::handleArgumentList(
    ast::NExpression* expr,
    ast::NArguments* next
) {
    ast::NArguments* args = new ast::NArguments(expr);
    if (next) {
        args->next = next;
    }
    return args;
}

// Initializers
ast::NInitializerList* ParserActions::handleInitializerList(
    ast::NExpression* expr,
    ast::NInitializerList* next
) {
    ast::NInitializerList* initList = new ast::NInitializerList();
    initList->push_back(expr);
    if (next) {
        // Merge elements from next list
        for (auto* elem : next->getElements()) {
            initList->push_back(elem);
        }
    }
    return initList;
}

// Expression Statement
ast::NExpressionStatement* ParserActions::handleExpressionStatement(ast::NExpression* expr) {
    return new ast::NExpressionStatement(expr);
}

// Type Specifiers
ast::TypeDescriptor* ParserActions::handlePointerType(
    ast::TypeDescriptor* baseType,
    int pointerLevel
) {
    if (pointerLevel <= 0) {
        return baseType;
    }
    // PointerTypeDescriptor expects unique_ptr, need to wrap
    ast::TypeDescriptorPtr basePtr(baseType);
    return new ast::PointerTypeDescriptor(std::move(basePtr), pointerLevel);
}

// Struct
ast::NStructDeclaration* ParserActions::handleStructDeclaration(
    ast::TypeDescriptor* type,
    ast::NDeclarator* declarator
) {
    return new ast::NStructDeclaration(type, declarator);
}

ast::NStructDeclaration* ParserActions::handleStructDeclarationList(
    ast::NStructDeclaration* current,
    ast::NStructDeclaration* next
) {
    if (current) {
        current->next = next;
    }
    return current;
}

ast::TypeDescriptor* ParserActions::handleStructSpecifier(
    const std::string& name,
    ast::NStructDeclaration* declarations
) {
    return new ast::StructTypeDescriptor(name, declarations);
}

ast::TypeDescriptor* ParserActions::handleStructReference(const std::string& name) {
    return new ast::StructTypeDescriptor(name, nullptr);
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

} // namespace toyc::semantic
