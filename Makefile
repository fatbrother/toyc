SRCDIR = src
INCDIR = include
BUILDDIR = build

SOURCES = $(shell find $(SRCDIR) -name "*.cpp")
INCLUDES = $(shell find $(INCDIR) -name "*.hpp")
SRC_OBJS = $(SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
GENERATED_OBJS = $(BUILDDIR)/lex.yy.o $(BUILDDIR)/y.tab.o
OBJS = $(GENERATED_OBJS) $(SRC_OBJS)

CXX = g++
LEX = lex
YACC = yacc
LLVM_CXXFLAGS = $(shell llvm-config --includedir)
LLVM_LDFLAGS = $(shell llvm-config --ldflags)
LLVM_LIB = $(shell llvm-config --libs)

FLAGS = -I$(INCDIR) -I$(LLVM_CXXFLAGS) -std=c++17
LDFLAGS = $(LLVM_LDFLAGS) $(LLVM_LIB)

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

$(BUILDDIR)/y.tab.o: $(BUILDDIR)/y.tab.cpp
	$(CXX) $(FLAGS) -c $< -o $@ $(LDFLAGS)

$(BUILDDIR)/lex.yy.o: $(BUILDDIR)/lex.yy.cpp $(BUILDDIR)/y.tab.hpp
	$(CXX) $(FLAGS) -c $< -o $@ $(LDFLAGS)

toyc: $(OBJS)
	$(CXX) $(FLAGS) $(OBJS) -o $@ $(LDFLAGS)

clean:
	rm -rf $(BUILDDIR) toyc