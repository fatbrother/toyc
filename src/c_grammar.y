%{
#include <string>
#include <iostream>
// #include <unordered_map>
#include "parser/y.tab.hpp"

void yyerror(const char *s);
extern "C" int yylex(void);
extern int yyparse();

// extern std::unordered_map<std::string, int> symbol_table;
extern int yylineno;
extern int yycolumn;
extern char* yytext;

%}

%union
{
	std::string* string;
	int token;
}


%token	<string> IDENTIFIER I_CONSTANT F_CONSTANT STRING_LITERAL
%token	PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token	AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token	SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token	XOR_ASSIGN OR_ASSIGN
%token	TYPEDEF_NAME

%token	TYPEDEF SIZEOF
%token	BOOL CHAR SHORT INT LONG FLOAT DOUBLE VOID
%token	SIGNED UNSIGNED CONST STATIC
%token	STRUCT ELLIPSIS

%token	CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%type	<string> constant

%start program
%%

program
	: program external_declaration
	| external_declaration
	;

external_declaration
	: function_definition
	;

function_definition
	: type IDENTIFIER '(' parameter_list ')' compound_statement
	| type IDENTIFIER '(' ')' compound_statement
	| type IDENTIFIER '(' parameter_list ')' ';'
	| type IDENTIFIER '(' ')' ';'
	;

parameter_list
	: parameter_list ',' parameter_declaration
	| parameter_declaration
	;

parameter_declaration
	: type IDENTIFIER
	| type IDENTIFIER '=' constant
	;

compound_statement
	: '{' statement_list '}'
	| '{' '}'
	;

statement_list
	: statement_list statement
	| statement
	;

statement
	: jump_statement
	| expression_statement
	| declaration_statement
	;

jump_statement
	: BREAK ';'
	| CONTINUE ';'
	| RETURN expression ';' {
		std::cout << "RETURN" << std::endl;
	}
	| RETURN ';'
	;

declaration_statement
	: declaration ';'
	;

expression_statement
	: expression ';'
	| ';'
	;

expression
	: constant
	;

declaration
	: type declarator_list
	;

declarator_list
	: declarator_list ',' declarator
	| declarator
	;

declarator
	: IDENTIFIER {
		std::cout << "IDENTIFIER: " << *$1 << std::endl;
	}
	| IDENTIFIER '=' expression
	;

constant
	: I_CONSTANT {
		std::cout << "I_CONSTANT: " << yytext << std::endl;
		$$ = new std::string(yytext);
	}
	| F_CONSTANT {
		std::cout << "F_CONSTANT: " << yytext << std::endl;
		$$ = new std::string(yytext);
	}
	| STRING_LITERAL {
		std::cout << "STRING_LITERAL: " << yytext << std::endl;
		$$ = new std::string(yytext);
	}
	;

type
	: TYPEDEF_NAME
	| BOOL
	| CHAR
	| SHORT
	| INT
	| LONG
	| FLOAT
	| DOUBLE
	| VOID
	;

%%
#include <stdio.h>

void yyerror(const char *s)
{
	std::cerr << yylineno << ":" << yycolumn << " " << "Error: " << s << " \"" << yytext << "\"" << std::endl;
}

/* void insert_symbol(const std::string &s, int type)
{
	symbol_table[s] = type;
} */