SRCDIR = src
INCDIR = include
BUILDDIR = build
TESTDIR = tests

SOURCES = $(shell find $(SRCDIR) -name "*.cpp")
INCLUDES = $(shell find $(INCDIR) -name "*.hpp")
SRC_OBJS = $(SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
GENERATED_OBJS = $(BUILDDIR)/lex.yy.o $(BUILDDIR)/y.tab.o
OBJS = $(GENERATED_OBJS) $(SRC_OBJS)

# Test-specific variables
TEST_SOURCES = $(shell find $(TESTDIR) -name "*.cpp")
TEST_OBJS = $(TEST_SOURCES:$(TESTDIR)/%.cpp=$(BUILDDIR)/tests/%.o)
# Exclude main.cpp for tests (we need our own main in test files)
LIB_OBJS = $(filter-out $(BUILDDIR)/toyc.o, $(SRC_OBJS)) $(GENERATED_OBJS)

CXX = g++
LEX = lex
YACC = bison
LLVM_VERSION = 18
LLVM_CONFIG = llvm-config-$(LLVM_VERSION)
LLVM_CXXFLAGS = $(shell $(LLVM_CONFIG) --includedir)
LLVM_LDFLAGS = $(shell $(LLVM_CONFIG) --ldflags)
LLVM_LIB = $(shell $(LLVM_CONFIG) --libs)

FLAGS = -g -I$(INCDIR) -I$(LLVM_CXXFLAGS) -std=c++17
LDFLAGS = $(LLVM_LDFLAGS) $(LLVM_LIB)
TEST_FLAGS = $(FLAGS) -lgtest -lgmock -pthread
TEST_LDFLAGS = $(LDFLAGS) -lgtest -lgmock -pthread

all: toyc

$(BUILDDIR)/lex.yy.cpp: $(SRCDIR)/c_lexer.l
	@mkdir -p $(BUILDDIR)
	$(LEX) -o $@ $<

$(BUILDDIR)/y.tab.cpp $(BUILDDIR)/y.tab.hpp: $(SRCDIR)/c_grammar.y
	@mkdir -p $(BUILDDIR)
	$(YACC) -d -o $(BUILDDIR)/y.tab.cpp $<

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp $(INCLUDES)
	@mkdir -p $(dir $@)
	$(CXX) $(FLAGS) -c $< -o $@ $(LDFLAGS)

$(BUILDDIR)/tests/%.o: $(TESTDIR)/%.cpp $(INCLUDES)
	@mkdir -p $(dir $@)
	$(CXX) $(TEST_FLAGS) -c $< -o $@

$(BUILDDIR)/y.tab.o: $(BUILDDIR)/y.tab.cpp
	$(CXX) $(FLAGS) -c $< -o $@ $(LDFLAGS)

$(BUILDDIR)/lex.yy.o: $(BUILDDIR)/lex.yy.cpp $(BUILDDIR)/y.tab.hpp
	$(CXX) $(FLAGS) -c $< -o $@ $(LDFLAGS)

toyc: $(OBJS)
	$(CXX) $(FLAGS) $(OBJS) -o $@ $(LDFLAGS)

# Test targets
test-build: $(TEST_OBJS) $(LIB_OBJS) toyc
	@mkdir -p $(BUILDDIR)/tests
	$(CXX) $(TEST_FLAGS) $(BUILDDIR)/tests/main_test.o $(BUILDDIR)/tests/test_preprocessor.o $(BUILDDIR)/tests/test_error_handler.o $(BUILDDIR)/tests/test_syntax.o $(BUILDDIR)/tests/test_output.o $(BUILDDIR)/tests/test_compiler_errors.o $(LIB_OBJS) -o $(BUILDDIR)/tests/all_tests $(TEST_LDFLAGS)

test: test-build
	$(BUILDDIR)/tests/all_tests

clean:
	rm -rf $(BUILDDIR) toyc

.PHONY: all test clean