#pragma once

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <stack>

#include "ast/node.hpp"
#include "ast/external_definition.hpp"
#include "utility/error_handler.hpp"

namespace toyc::semantic {

/**
 * @brief Symbol information in the symbol table
 */
struct Symbol {
    std::string name;
    ast::TypeDescriptor* type;
    bool isFunction;
    bool isDefined;
    int scopeLevel;
    
    Symbol() : type(nullptr), isFunction(false), isDefined(false), scopeLevel(0) {}
    Symbol(const std::string& n, ast::TypeDescriptor* t, bool isFn = false, bool isDef = true, int level = 0)
        : name(n), type(t), isFunction(isFn), isDefined(isDef), scopeLevel(level) {}
};

/**
 * @brief Semantic analyzer for ToyC compiler
 * 
 * This class is responsible for:
 * - Type checking
 * - Symbol table management
 * - Scope management
 * - Semantic error detection
 */
class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    ~SemanticAnalyzer();

    /**
     * @brief Analyze the entire program AST
     * @param program Root of the AST
     * @return true if analysis succeeds, false otherwise
     */
    bool analyze(ast::NExternalDeclaration* program);

    /**
     * @brief Get the error handler
     * @return Pointer to error handler (may be nullptr if no errors)
     */
    utility::ErrorHandler* getErrorHandler() const { return errorHandler; }

    /**
     * @brief Check if there are any errors
     */
    bool hasErrors() const { return errorHandler != nullptr; }

    /**
     * @brief Clear error handler
     */
    void clearError();

private:
    // Symbol table: map from symbol name to symbol info
    // We use a stack of maps to handle nested scopes
    std::vector<std::unordered_map<std::string, Symbol>> symbolTable;
    int currentScopeLevel;

    // Error handler
    utility::ErrorHandler* errorHandler;

    // Scope management
    void enterScope();
    void exitScope();

    // Symbol table operations
    bool declareSymbol(const std::string& name, ast::TypeDescriptor* type, bool isFunction = false, bool isDefined = true);
    Symbol* lookupSymbol(const std::string& name);
    Symbol* lookupSymbolInCurrentScope(const std::string& name);

    // Analysis methods for different AST nodes
    bool analyzeExternalDeclaration(ast::NExternalDeclaration* decl);
    bool analyzeFunctionDefinition(ast::NFunctionDefinition* funcDef);
    bool analyzeDeclarationStatement(ast::NDeclarationStatement* declStmt);
    bool analyzeStatement(ast::NStatement* stmt);
    bool analyzeBlock(ast::NBlock* block);
    bool analyzeExpression(ast::NExpression* expr);

    // Type checking
    ast::TypeDescriptor* getExpressionType(ast::NExpression* expr);
    bool areTypesCompatible(ast::TypeDescriptor* type1, ast::TypeDescriptor* type2);

    // Error reporting
    void reportError(const std::string& message, int line = 0, int column = 0);
};

} // namespace toyc::semantic
