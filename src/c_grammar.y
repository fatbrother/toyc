%{
#include <string>
#include "ast/node.hpp"
#include "ast/expression.hpp"
#include "ast/statement.hpp"
#include "ast/external_definition.hpp"
#include "parser/y.tab.hpp"

// #include <unordered_map>

void yyerror(const char *s);
extern "C" int yylex(void);
extern int yyparse();

// extern std::unordered_map<std::string, int> symbol_table;
extern int yylineno;
extern int yycolumn;
extern char* yytext;

toyc::ast::NExternalDeclaration *program;

%}

%union
{
	toyc::ast::NExpression *expression;
	toyc::ast::NType *type;
	toyc::ast::NDeclarator *declarator;
	toyc::ast::NStatement *statement;
	toyc::ast::NExternalDeclaration *external_declaration;
	toyc::ast::NParameter *parameter;
	char *string;
	int token;
}


%token	<string> IDENTIFIER I_CONSTANT F_CONSTANT STRING_LITERAL TYPEDEF_NAME
%token	<token> PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP L_OP G_OP
%token	<token> ADD_OP SUB_OP MUL_OP DIV_OP MOD_OP AND_OP OR_OP XOR_OP BIT_AND_OP BIT_OR_OP
%token	MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token	SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token	XOR_ASSIGN OR_ASSIGN

%token	TYPEDEF SIZEOF
%token	<token> BOOL CHAR SHORT INT LONG FLOAT DOUBLE VOID
%token	SIGNED UNSIGNED CONST STATIC
%token	STRUCT ELLIPSIS

%token	CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%type	<expression> expression condition calculation term factor numeric string
%type   <type> type
%type	<token> condition_op
%type   <declarator> declarator declarator_list
%type   <parameter> parameter_list parameter_declaration
%type   <statement> statement statement_list declaration_statement expression_statement compound_statement jump_statement
%type   <external_declaration> program external_declaration external_declaration_list function_definition

%start program
%%

program
	: external_declaration_list {
		program = $1;
	}
	;

external_declaration_list
	: external_declaration external_declaration_list {
		$$ = $1;
		$$->next = $2;
	}
	| external_declaration {
		$$ = $1;
	}
	;

external_declaration
	: function_definition {
		$$ = $1;
	}
	;

function_definition
	: type IDENTIFIER '(' parameter_list ')' compound_statement {
		$$ = new toyc::ast::NFunctionDefinition($1, std::string($2), $4, (toyc::ast::NBlock *)$6);
		delete $2;
	}
	| type IDENTIFIER '(' ')' compound_statement {
		$$ = new toyc::ast::NFunctionDefinition($1, std::string($2), nullptr, (toyc::ast::NBlock *)$5);
		delete $2;
	}
	| type IDENTIFIER '(' parameter_list ')' ';' {
		$$ = new toyc::ast::NFunctionDefinition($1, std::string($2), $4, nullptr);
		delete $2;
	}
	| type IDENTIFIER '(' VOID ')' compound_statement {
		$$ = new toyc::ast::NFunctionDefinition($1, std::string($2), nullptr, (toyc::ast::NBlock *)$6);
		delete $2;
	}
	| type IDENTIFIER '(' VOID ')' ';' {
		$$ = new toyc::ast::NFunctionDefinition($1, std::string($2), nullptr, nullptr);
		delete $2;
	}
	| type IDENTIFIER '(' ')' ';' {
		$$ = new toyc::ast::NFunctionDefinition($1, std::string($2), nullptr, nullptr);
		delete $2;
	}
	;

parameter_list
	: parameter_declaration ',' parameter_list {
		$$ = $1;
		$$->next = $3;
	}
	| parameter_declaration {
		$$ = $1;
	}
	;

parameter_declaration
	: type IDENTIFIER {
		$$ = new toyc::ast::NParameter($1, std::string($2));
		delete $2;
	}
	;

compound_statement
	: '{' statement_list '}' {
		$$ = new toyc::ast::NBlock($2);
	}
	| '{' '}' {
		$$ = new toyc::ast::NBlock();
	}
	;

statement_list
	: statement statement_list {
		$$ = $1;
		$$->next = $2;
	}
	| statement {
		$$ = $1;
	}
	;

statement
	: expression_statement {
		$$ = $1;
	}
	| declaration_statement {
		$$ = $1;
	}
	| compound_statement {
		$$ = $1;
	}
	| jump_statement {
		$$ = $1;
	}
	;

jump_statement
	: RETURN expression ';' {
		$$ = new toyc::ast::NReturnStatement($2);
	}
	| RETURN ';' {
		$$ = new toyc::ast::NReturnStatement(nullptr);
	}
	/* | BREAK ';'
	| CONTINUE ';' */
	;

declaration_statement
	: type declarator_list ';' {
		$$ = new toyc::ast::NDeclarationStatement($1, $2);
	}
	;

declarator
	: IDENTIFIER {
		$$ = new toyc::ast::NDeclarator(std::string($1), nullptr);
		delete $1;
	}
	| IDENTIFIER '=' expression {
		$$ = new toyc::ast::NDeclarator(std::string($1), $3);
		delete $1;
	}
	;

declarator_list
	: declarator ',' declarator_list {
		$$ = $1;
		$$->next = $3;
	}
	| declarator {
		$$ = $1;
	}
	;

expression_statement
	: expression ';' {
		$$ = new toyc::ast::NExpressionStatement($1);
	}
	| ';' {
		$$ = new toyc::ast::NExpressionStatement(nullptr);
	}
	;

expression:
      condition AND_OP expression {
        $$ = new toyc::ast::NBinaryOperator($1, $2, $3);
      }
    | condition OR_OP expression {
        $$ = new toyc::ast::NBinaryOperator($1, $2, $3);
      }
    | condition {
        $$ = $1;
      }
    ;

condition:
      calculation condition_op calculation {
        $$ = new toyc::ast::NBinaryOperator($1, $2, $3);
      }
    | calculation {
        $$ = $1;
      }
    ;

calculation:
      term {
        $$ = $1;
      }
    | calculation ADD_OP term {
        $$ = new toyc::ast::NBinaryOperator($1, $2, $3);
      }
    | calculation SUB_OP term {
        $$ = new toyc::ast::NBinaryOperator($1, $2, $3);
      }
    ;

term:
      factor {
        $$ = $1;
      }
    | term MUL_OP factor {
        $$ = new toyc::ast::NBinaryOperator($1, $2, $3);
      }
    | term DIV_OP factor {
        $$ = new toyc::ast::NBinaryOperator($1, $2, $3);
      }
	| term MOD_OP factor {
		$$ = new toyc::ast::NBinaryOperator($1, $2, $3);
	  }
    ;

factor:
	  '(' expression ')' {
		$$ = $2;
	  }
	| IDENTIFIER {
		$$ = new toyc::ast::NIdentifier(std::string($1));
		delete $1;
	  }
    | numeric {
        $$ = $1;
      }
    | string {
        $$ = $1;
      }

numeric:
      I_CONSTANT {
        $$ = new toyc::ast::NInteger(atoi($1));
		delete $1;
      }
    | F_CONSTANT {
		$$ = new toyc::ast::NFloat(atof($1));
		delete $1;
	  }
    ;

string:
      STRING_LITERAL {
		std::string str($1);
        $$ = new toyc::ast::NString(str.substr(1, str.length() - 2));
		delete $1;
      }
    ;


condition_op
	: LE_OP
	| GE_OP
	| EQ_OP
	| NE_OP
	| AND_OP
	| OR_OP
	| L_OP
	| G_OP
	;

type
	: TYPEDEF_NAME {
		$$ = new toyc::ast::NType(TYPEDEF_NAME, std::string($1));
		delete $1;
	}
	| BOOL {
		$$ = new toyc::ast::NType($1);
	}
	| CHAR {
		$$ = new toyc::ast::NType($1);
	}
	| SHORT {
		$$ = new toyc::ast::NType($1);
	}
	| INT {
		$$ = new toyc::ast::NType($1);
	}
	| LONG {
		$$ = new toyc::ast::NType($1);
	}
	| FLOAT {
		$$ = new toyc::ast::NType($1);
	}
	| DOUBLE {
		$$ = new toyc::ast::NType($1);
	}
	| VOID {
		$$ = new toyc::ast::NType($1);
	}
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