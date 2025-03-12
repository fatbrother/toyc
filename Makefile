SRCDIR = src
INCDIR = include
BUILDDIR = build

SOURCES = $(wildcard $(SRCDIR)/*.cpp)
GENERATED_S/sources = $(BUILDDIR)/lex.yy.c $(BUILDDIR)/y.tab.c
ALL_S/sources = $(SOURCES) $(GENERATED_S/sources)

SRC_OBJS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))
GENERATED_OBJS = $(BUILDDIR)/lex.yy.o $(BUILDDIR)/y.tab.o
OBJS = $(SRC_OBJS) $(GENERATED_OBJS)

CXX = g++
LEX = lex
YACC = yacc
FLAGS = -I$(INCDIR)

# Define LLVM library name (adjust based on installed version)
LLVM_LIB = -lLLVM-14

all: toyc

$(BUILDDIR)/lex.yy.c: $(SRCDIR)/c_lexer.l
	@mkdir -p $(BUILDDIR)
	$(LEX) -o $@ $<

$(BUILDDIR)/y.tab.c $(BUILDDIR)/y.tab.h: $(SRCDIR)/c_grammar.y
	@mkdir -p $(BUILDDIR)
	$(YACC) -d -o $(BUILDDIR)/y.tab.c $<

$(BUILDDIR)/y.tab.o: $(BUILDDIR)/y.tab.c
	$(CXX) $(FLAGS) -c $< -o $@

$(BUILDDIR)/lex.yy.o: $(BUILDDIR)/lex.yy.c $(BUILDDIR)/y.tab.h
	$(CXX) $(FLAGS) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(FLAGS) -c $< -o $@

toyc: $(OBJS)
	$(CXX) $(OBJS) -o $@ -lfl $(LLVM_LIB)

clean:
	rm -rf $(BUILDDIR) toyc