#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "utility/preprocessor.hpp"
#include <sstream>
#include <fstream>

using namespace toyc::utility;
using ::testing::HasSubstr;
using ::testing::Not;

class PreprocessorTest : public ::testing::Test {
protected:
    void SetUp() override {
        preprocessor = std::make_unique<Preprocessor>();
        // Add fixtures directory to include paths
        preprocessor->addIncludePath("tests/fixtures/preprocessor");
    }

    void TearDown() override {
        // No cleanup needed for fixture files
    }

    std::unique_ptr<Preprocessor> preprocessor;

    // Helper function to read file content
    std::string readFile(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
};

// Test basic object macro expansion
TEST_F(PreprocessorTest, BasicObjectMacroExpansion) {
    std::string input = readFile("tests/fixtures/preprocessor/basic_macro.c");
    std::string result = preprocessor->preprocessContent(input, "basic_macro.c");

    EXPECT_THAT(result, HasSubstr("int size = 100;"));
    EXPECT_THAT(result, Not(HasSubstr("MAX_SIZE")));
}

// Test function macro expansion
TEST_F(PreprocessorTest, FunctionMacroExpansion) {
    std::string input = readFile("tests/fixtures/preprocessor/function_macro.c");
    std::string result = preprocessor->preprocessContent(input, "function_macro.c");

    EXPECT_THAT(result, HasSubstr("int result = ((5) * (5));"));
    EXPECT_THAT(result, Not(HasSubstr("SQUARE")));
}

// Test nested macro expansion
TEST_F(PreprocessorTest, NestedMacroExpansion) {
    std::string input = readFile("tests/fixtures/preprocessor/nested_macro.c");
    std::string result = preprocessor->preprocessContent(input, "nested_macro.c");

    EXPECT_THAT(result, HasSubstr("int result = ((10) * (10));"));
}

// Test conditional compilation - ifdef
TEST_F(PreprocessorTest, ConditionalIfdef) {
    std::string input = readFile("tests/fixtures/preprocessor/conditional_ifdef.c");
    std::string result = preprocessor->preprocessContent(input, "conditional_ifdef.c");

    EXPECT_THAT(result, HasSubstr("int debug_mode = 1;"));
}

// Test conditional compilation - ifndef
TEST_F(PreprocessorTest, ConditionalIfndef) {
    std::string input = readFile("tests/fixtures/preprocessor/conditional_ifndef.c");
    std::string result = preprocessor->preprocessContent(input, "conditional_ifndef.c");

    EXPECT_THAT(result, HasSubstr("int debug_mode = 1;"));
}

// Test conditional compilation - if with expression
TEST_F(PreprocessorTest, ConditionalIfExpression) {
    std::string input = readFile("tests/fixtures/preprocessor/conditional_if_expression.c");
    std::string result = preprocessor->preprocessContent(input, "conditional_if_expression.c");

    EXPECT_THAT(result, HasSubstr("int new_feature = 1;"));
}

// Test conditional compilation - else
TEST_F(PreprocessorTest, ConditionalElse) {
    std::string input = readFile("tests/fixtures/preprocessor/conditional_else.c");
    std::string result = preprocessor->preprocessContent(input, "conditional_else.c");

    EXPECT_THAT(result, HasSubstr("int a = 2;"));
    EXPECT_THAT(result, Not(HasSubstr("int a = 1;")));
}

// Test undef directive
TEST_F(PreprocessorTest, UndefDirective) {
    std::string input = readFile("tests/fixtures/preprocessor/undef_directive.c");
    std::string result = preprocessor->preprocessContent(input, "undef_directive.c");

    EXPECT_THAT(result, HasSubstr("int value = TEST;")); // TEST should not be expanded
}

// Test include directive
TEST_F(PreprocessorTest, IncludeDirective) {
    std::string input = readFile("tests/fixtures/preprocessor/include_test.c");
    std::string result = preprocessor->preprocessContent(input, "include_test.c");

    EXPECT_THAT(result, HasSubstr("extern int global_var;"));
    EXPECT_THAT(result, HasSubstr("int value = 42;"));
}

// Test predefined macros
TEST_F(PreprocessorTest, PredefinedMacros) {
    std::string input = readFile("tests/fixtures/preprocessor/predefined_macros.c");
    std::string result = preprocessor->preprocessContent(input, "predefined_macros.c");

    EXPECT_THAT(result, HasSubstr("int line = 1;"));
    EXPECT_THAT(result, HasSubstr("char* file = \"predefined_macros.c\";"));
}

// Test macro with empty body
TEST_F(PreprocessorTest, EmptyMacro) {
    std::string input = readFile("tests/fixtures/preprocessor/empty_macro.c");
    std::string result = preprocessor->preprocessContent(input, "empty_macro.c");

    EXPECT_THAT(result, HasSubstr("int  value  =  1 ;"));
}

// Test function macro with multiple parameters
TEST_F(PreprocessorTest, FunctionMacroMultipleParams) {
    std::string input = readFile("tests/fixtures/preprocessor/function_macro_multiple_params.c");
    std::string result = preprocessor->preprocessContent(input, "function_macro_multiple_params.c");

    EXPECT_THAT(result, HasSubstr("int result = ((10) > (20) ? (10) : (20));"));
}

// Test function macro with no parameters
TEST_F(PreprocessorTest, FunctionMacroNoParams) {
    std::string input = readFile("tests/fixtures/preprocessor/function_macro_no_params.c");
    std::string result = preprocessor->preprocessContent(input, "function_macro_no_params.c");

    EXPECT_THAT(result, HasSubstr("int value = 0;"));
}

// Test line continuation
TEST_F(PreprocessorTest, LineContinuation) {
    std::string input = readFile("tests/fixtures/preprocessor/line_continuation.c");
    std::string result = preprocessor->preprocessContent(input, "line_continuation.c");

    EXPECT_THAT(result, HasSubstr("this is a     long macro"));
}

// Test comments are ignored
TEST_F(PreprocessorTest, CommentsIgnored) {
    std::string input = readFile("tests/fixtures/preprocessor/comments_ignored.c");
    std::string result = preprocessor->preprocessContent(input, "comments_ignored.c");

    EXPECT_THAT(result, HasSubstr("int x = 42;"));
    EXPECT_THAT(result, HasSubstr("// This is a comment"));
}

// Test nested conditional compilation
TEST_F(PreprocessorTest, NestedConditionals) {
    std::string input = readFile("tests/fixtures/preprocessor/nested_conditionals.c");
    std::string result = preprocessor->preprocessContent(input, "nested_conditionals.c");

    EXPECT_THAT(result, HasSubstr("int value = 1;"));
}

// Test elif directive
TEST_F(PreprocessorTest, ElifDirective) {
    std::string input = readFile("tests/fixtures/preprocessor/elif_test.c");
    std::string result = preprocessor->preprocessContent(input, "elif_test.c");

    EXPECT_THAT(result, HasSubstr("int v2 = 2;"));
    EXPECT_THAT(result, Not(HasSubstr("int v1 = 1;")));
    EXPECT_THAT(result, Not(HasSubstr("int v3 = 3;")));
}

// Test defined() operator
TEST_F(PreprocessorTest, DefinedOperator) {
    std::string input = readFile("tests/fixtures/preprocessor/defined_operator.c");
    std::string result = preprocessor->preprocessContent(input, "defined_operator.c");

    EXPECT_THAT(result, HasSubstr("int value = 1;"));
}

// Test macro redefinition (should work)
TEST_F(PreprocessorTest, MacroRedefinition) {
    std::string input = readFile("tests/fixtures/preprocessor/macro_redefinition.c");
    std::string result = preprocessor->preprocessContent(input, "macro_redefinition.c");

    EXPECT_THAT(result, HasSubstr("int x = 2;"));
}

// Test error cases
TEST_F(PreprocessorTest, InvalidDirectives) {
    std::string input1 = readFile("tests/fixtures/preprocessor/invalid_directives.c");

    // Capture stderr to check for error messages
    testing::internal::CaptureStderr();
    std::string result1 = preprocessor->preprocessContent(input1, "invalid_directives.c");
    std::string stderr_output = testing::internal::GetCapturedStderr();

    // Should contain error message about missing #endif
    EXPECT_THAT(stderr_output, HasSubstr("error"));
}

// Test complex real-world scenario
TEST_F(PreprocessorTest, ComplexScenario) {
    preprocessor->addPredefinedMacro("MATH_EXTENDED", "1");

    std::string input = readFile("tests/fixtures/preprocessor/complex_test.c");
    std::string result = preprocessor->preprocessContent(input, "complex_test.c");

    EXPECT_THAT(result, HasSubstr("double area = (3.14159 * ((5) * (5)));"));
}

// Test user-defined macros via command line
TEST_F(PreprocessorTest, UserDefinedMacros) {
    preprocessor->addPredefinedMacro("USER_MACRO", "100");

    std::string input = readFile("tests/fixtures/preprocessor/user_defined_macros.c");
    std::string result = preprocessor->preprocessContent(input, "user_defined_macros.c");

    EXPECT_THAT(result, HasSubstr("int value = 100;"));
}

// Test include paths
TEST_F(PreprocessorTest, IncludePaths) {
    std::string input = readFile("tests/fixtures/preprocessor/include_path_test.c");
    std::string result = preprocessor->preprocessContent(input, "include_path_test.c");

    EXPECT_THAT(result, HasSubstr("int x = 42;"));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
