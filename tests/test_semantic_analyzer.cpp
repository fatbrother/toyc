#include <gtest/gtest.h>
#include <memory>
#include "semantic/semantic_analyzer.hpp"
#include "ast/node.hpp"
#include "ast/external_definition.hpp"
#include "ast/statement.hpp"
#include "ast/expression.hpp"
#include "ast/type.hpp"

using namespace toyc::semantic;
using namespace toyc::ast;

class SemanticAnalyzerTest : public ::testing::Test {
protected:
    SemanticAnalyzer* analyzer;

    void SetUp() override {
        analyzer = new SemanticAnalyzer();
    }

    void TearDown() override {
        delete analyzer;
    }
};

// Test: Empty program should fail
TEST_F(SemanticAnalyzerTest, EmptyProgram) {
    bool result = analyzer->analyze(nullptr);
    EXPECT_FALSE(result);
    EXPECT_TRUE(analyzer->hasErrors());
}

// Test: Simple function definition
TEST_F(SemanticAnalyzerTest, SimpleFunctionDefinition) {
    // Create a simple function: int main() { }
    TypeDescriptor* returnType = new PrimitiveTypeDescriptor(VarType::VAR_TYPE_INT);
    NBlock* body = new NBlock();
    NFunctionDefinition* funcDef = new NFunctionDefinition(returnType, "main", nullptr, body);

    bool result = analyzer->analyze(funcDef);
    EXPECT_TRUE(result);
    EXPECT_FALSE(analyzer->hasErrors());

    delete funcDef;
}

// Test: Error handler is set on error
TEST_F(SemanticAnalyzerTest, ErrorHandlerSetOnError) {
    // Analyze null program - should set error
    analyzer->analyze(nullptr);
    
    EXPECT_TRUE(analyzer->hasErrors());
    EXPECT_NE(analyzer->getErrorHandler(), nullptr);
    
    if (analyzer->getErrorHandler()) {
        std::string errorMsg = analyzer->getErrorHandler()->getMessage();
        EXPECT_FALSE(errorMsg.empty());
    }
}

// Test: Clear error handler
TEST_F(SemanticAnalyzerTest, ClearErrorHandler) {
    // Generate an error
    analyzer->analyze(nullptr);
    EXPECT_TRUE(analyzer->hasErrors());
    
    // Clear the error
    analyzer->clearError();
    EXPECT_FALSE(analyzer->hasErrors());
    EXPECT_EQ(analyzer->getErrorHandler(), nullptr);
}
