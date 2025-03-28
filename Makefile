SRCDIR = src
INCDIR = include
BUILDDIR = build

SOURCES = $(shell find $(SRCDIR) -name "*.cpp")
$(info SOURCES = $(SOURCES))
SRC_OBJS = $(patsubst $(SRCDIR)/*.cpp,$(BUILDDIR)/*.o,$(SOURCES))
GENERATED_OBJS = $(BUILDDIR)/lex.yy.o $(BUILDDIR)/y.tab.o
OBJS = $(SRC_OBJS) $(GENERATED_OBJS)

CXX = g++
LEX = lex
YACC = yacc
LLVM_LIB = $(shell llvm-config --ldflags --libs)
FLAGS = -I$(INCDIR) $(LLVM_LIB)

all: toyc

$(BUILDDIR)/lex.yy.cpp: $(SRCDIR)/c_lexer.l
	@mkdir -p $(BUILDDIR)
	$(LEX) -o $@ $<

$(BUILDDIR)/y.tab.cpp $(BUILDDIR)/y.tab.hpp: $(SRCDIR)/c_grammar.y
	@mkdir -p $(BUILDDIR)
	$(YACC) -d -o $(BUILDDIR)/y.tab.cpp $<
	cp $(BUILDDIR)/y.tab.hpp $(INCDIR)/parser/

$(BUILDDIR)/y.tab.o: $(BUILDDIR)/y.tab.cpp
	$(CXX) $(FLAGS) -c $< -o $@

$(BUILDDIR)/lex.yy.o: $(BUILDDIR)/lex.yy.cpp $(BUILDDIR)/y.tab.hpp
	$(CXX) $(FLAGS) -c $< -o $@

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(FLAGS) -c $< -o $@

toyc: $(OBJS)
	$(CXX) $(FLAGS) $(OBJS) -o $@ -lfl

clean:
	rm -rf $(BUILDDIR) toyc