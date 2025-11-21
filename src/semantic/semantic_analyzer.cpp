#include "semantic/semantic_analyzer.hpp"
#include "ast/expression.hpp"
#include "ast/statement.hpp"
#include "ast/external_definition.hpp"

#include <iostream>

namespace toyc::semantic {

SemanticAnalyzer::SemanticAnalyzer()
    : currentScopeLevel(0), errorHandler(nullptr) {
    // Initialize global scope
    symbolTable.push_back(std::unordered_map<std::string, Symbol>());
}

SemanticAnalyzer::~SemanticAnalyzer() {
    clearError();
}

void SemanticAnalyzer::clearError() {
    if (errorHandler != nullptr) {
        delete errorHandler;
        errorHandler = nullptr;
    }
}

bool SemanticAnalyzer::analyze(ast::NExternalDeclaration* program) {
    if (program == nullptr) {
        reportError("Empty program");
        return false;
    }

    // Traverse all external declarations
    for (auto* decl = program; decl != nullptr; decl = decl->next) {
        if (!analyzeExternalDeclaration(decl)) {
            return false;
        }
    }

    return true;
}

void SemanticAnalyzer::enterScope() {
    currentScopeLevel++;
    symbolTable.push_back(std::unordered_map<std::string, Symbol>());
}

void SemanticAnalyzer::exitScope() {
    if (currentScopeLevel > 0) {
        symbolTable.pop_back();
        currentScopeLevel--;
    }
}

bool SemanticAnalyzer::declareSymbol(const std::string& name, ast::TypeDescriptor* type,
                                      bool isFunction, bool isDefined) {
    // Check if symbol already exists in current scope
    Symbol* existing = lookupSymbolInCurrentScope(name);
    if (existing != nullptr) {
        // Allow function forward declaration
        if (isFunction && existing->isFunction && !existing->isDefined) {
            existing->isDefined = isDefined;
            return true;
        }
        reportError("Redeclaration of '" + name + "'");
        return false;
    }

    // Add symbol to current scope
    symbolTable[currentScopeLevel][name] = Symbol(name, type, isFunction, isDefined, currentScopeLevel);
    return true;
}

Symbol* SemanticAnalyzer::lookupSymbol(const std::string& name) {
    // Search from current scope to global scope
    for (int i = currentScopeLevel; i >= 0; i--) {
        auto it = symbolTable[i].find(name);
        if (it != symbolTable[i].end()) {
            return &(it->second);
        }
    }
    return nullptr;
}

Symbol* SemanticAnalyzer::lookupSymbolInCurrentScope(const std::string& name) {
    auto it = symbolTable[currentScopeLevel].find(name);
    if (it != symbolTable[currentScopeLevel].end()) {
        return &(it->second);
    }
    return nullptr;
}

bool SemanticAnalyzer::analyzeExternalDeclaration(ast::NExternalDeclaration* decl) {
    if (decl == nullptr) {
        return true;
    }

    // Check if it's a function definition
    if (decl->getType() == "FunctionDefinition") {
        return analyzeFunctionDefinition(static_cast<ast::NFunctionDefinition*>(decl));
    }
    // Check if it's a declaration statement (global variable)
    else if (decl->getType() == "DeclarationStatement") {
        return analyzeDeclarationStatement(static_cast<ast::NDeclarationStatement*>(decl));
    }

    return true;
}

bool SemanticAnalyzer::analyzeFunctionDefinition(ast::NFunctionDefinition* funcDef) {
    if (funcDef == nullptr) {
        return true;
    }

    // Get function name (this is a simplification; you'll need to implement getName())
    // For now, we'll skip detailed function analysis and focus on structure

    // Note: NFunctionDefinition has name as private member
    // We would need to add a getter or make it accessible
    // For this implementation, we'll declare it based on presence

    // Enter function scope
    enterScope();

    // Analyze parameters
    // Note: Would need getParams() method
    // auto* params = funcDef->getParams();
    // for (auto* param = params; param != nullptr; param = param->next) {
    //     if (!param->isVariadic) {
    //         declareSymbol(param->getName(), param->getTypeDescriptor(), false, true);
    //     }
    // }

    // Analyze function body
    // Note: Would need getBody() method
    // if (funcDef->getBody() != nullptr) {
    //     analyzeBlock(funcDef->getBody());
    // }

    // Exit function scope
    exitScope();

    return true;
}

bool SemanticAnalyzer::analyzeDeclarationStatement(ast::NDeclarationStatement* declStmt) {
    if (declStmt == nullptr) {
        return true;
    }

    // Note: NDeclarationStatement has private members
    // Would need getters to access typeDesc and declarator
    // For now, we just return true as structure is set up

    return true;
}

bool SemanticAnalyzer::analyzeStatement(ast::NStatement* stmt) {
    if (stmt == nullptr) {
        return true;
    }

    std::string type = stmt->getType();

    if (type == "Block") {
        return analyzeBlock(static_cast<ast::NBlock*>(stmt));
    } else if (type == "DeclarationStatement") {
        return analyzeDeclarationStatement(static_cast<ast::NDeclarationStatement*>(stmt));
    } else if (type == "ExpressionStatement") {
        auto* exprStmt = static_cast<ast::NExpressionStatement*>(stmt);
        // Would need getExpression() method
        // return analyzeExpression(exprStmt->getExpression());
        return true;
    } else if (type == "ReturnStatement") {
        // Analyze return statement
        return true;
    } else if (type == "IfStatement") {
        // Analyze if statement
        return true;
    } else if (type == "WhileStatement") {
        // Analyze while statement
        return true;
    } else if (type == "ForStatement") {
        // Analyze for statement
        return true;
    }

    // Handle other statement types
    return true;
}

bool SemanticAnalyzer::analyzeBlock(ast::NBlock* block) {
    if (block == nullptr) {
        return true;
    }

    enterScope();

    // Analyze all statements in the block
    auto* stmts = block->getStatements();
    for (auto* stmt = stmts; stmt != nullptr; stmt = stmt->next) {
        if (!analyzeStatement(stmt)) {
            exitScope();
            return false;
        }
    }

    exitScope();
    return true;
}

bool SemanticAnalyzer::analyzeExpression(ast::NExpression* expr) {
    if (expr == nullptr) {
        return true;
    }

    // Type checking would go here
    // For now, just return true

    return true;
}

ast::TypeDescriptor* SemanticAnalyzer::getExpressionType(ast::NExpression* expr) {
    // This would return the type of an expression
    // For now, return nullptr
    return nullptr;
}

bool SemanticAnalyzer::areTypesCompatible(ast::TypeDescriptor* type1, ast::TypeDescriptor* type2) {
    // This would check if two types are compatible
    // For now, return true
    return true;
}

void SemanticAnalyzer::reportError(const std::string& message, int line, int column) {
    if (errorHandler != nullptr) {
        delete errorHandler;
    }
    errorHandler = new utility::ErrorHandler(message, line, column);
}

} // namespace toyc::semantic
