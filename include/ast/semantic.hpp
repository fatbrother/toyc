#pragma once

#include "node.hpp"
#include <string>

// TODO: 將 c_grammar.y 中的語義動作封裝到這個類別中

class SemanticActions {
private:
    SymbolTable current_symbol_table_;
    Parser* parser_; // 指向 Parser 實例，用於報告錯誤

public:
    SemanticActions(Parser* p);

    // --- 封裝 Yacc 動作的方法 ---

    /**
     * @brief 處理函數定義 (function_definition 規則的動作)
     * @return FunctionNode* (AST 節點)
     */
    ASTNode* handleFunctionDefinition(
        const std::string& name,
        ASTNode* params,
        ASTNode* body,
        int line
    );

    /**
     * @brief 處理變數宣告 (variable_declaration 規則的動作)
     * @return VariableDeclNode* (AST 節點)
     */
    ASTNode* handleVariableDeclaration(
        const std::string& name,
        ASTNode* initializer,
        int line
    );

    /**
     * @brief 處理二元運算式 (expression PLUS expression 規則的動作)
     * @return BinaryOpNode* (AST 節點)
     */
    ASTNode* handleBinaryExpression(
        BinaryOp op,
        ASTNode* left,
        ASTNode* right
    );

    // --- 語義檢查輔助方法 ---
    void enterScope();
    void exitScope();
    ASTNode* lookupSymbol(const std::string& name);

    // 類型檢查等...
    // void checkTypeCompatibility(ASTNode* node1, ASTNode* node2);
};