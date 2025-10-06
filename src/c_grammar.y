%{
#include <string>
#include <iostream>

#include "ast/node.hpp"
#include "ast/expression.hpp"
#include "ast/statement.hpp"
#include "utility/error_handler.hpp"

// #include <unordered_map>

void yyerror(const char *s);
extern "C" int yylex(void);
extern int yyparse();

// extern std::unordered_map<std::string, int> symbol_table;
extern int yylineno;
extern int yycolumn;
extern char* yytext;
extern int yyleng;

toyc::ast::NExternalDeclaration *program;
toyc::utility::ErrorHandler *error_handler = nullptr;

%}

%union
{
	toyc::ast::NExpression *expression;
	toyc::ast::NType *type;
	toyc::ast::NDeclarator *declarator;
	toyc::ast::NStatement *statement;
	toyc::ast::NBlock *block;
	toyc::ast::NExternalDeclaration *external_declaration;
	toyc::ast::NParameter *parameter;
	toyc::ast::NArguments *arguments;
	toyc::ast::BineryOperator bop;
	std::string *string;
	int token;
}

%define parse.error verbose

%token	<string> IDENTIFIER I_CONSTANT F_CONSTANT STRING_LITERAL TYPEDEF_NAME
%token	INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP PTR_OP AND_OP OR_OP
%token	MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token	SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token	XOR_ASSIGN OR_ASSIGN

%token	TYPEDEF SIZEOF
%token	BOOL CHAR SHORT INT LONG FLOAT DOUBLE VOID
%token	SIGNED UNSIGNED CONST STATIC
%token	STRUCT ELLIPSIS

%token	CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%type	<expression> expression unary_expression assignment_expression relational_expression equality_expression additive_expression multiplicative_expression primary_expression numeric postfix_expression conditional_expression logical_or_expression logical_and_expression inclusive_or_expression exclusive_or_expression and_expression
%type   <type> type
%type	<bop> relational_expression_op
%type   <declarator> declarator declarator_list
%type   <parameter> parameter_list parameter_declaration
%type   <statement> statement statement_list declaration_statement expression_statement jump_statement for_statement_init_declaration
%type   <statement>  if_statement for_statement while_statement do_while_statement
%type   <block> compound_statement
%type   <external_declaration> program external_declaration external_declaration_list function_definition
%type   <arguments> argument_expression_list

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
		$$ = new toyc::ast::NFunctionDefinition($1, *$2, $4, $6);
		delete $2;
	}
	| type IDENTIFIER '(' ')' compound_statement {
		$$ = new toyc::ast::NFunctionDefinition($1, *$2, nullptr, $5);
		delete $2;
	}
	| type IDENTIFIER '(' parameter_list ')' ';' {
		$$ = new toyc::ast::NFunctionDefinition($1, *$2, $4, nullptr);
		delete $2;
	}
	| type IDENTIFIER '(' VOID ')' compound_statement {
		$$ = new toyc::ast::NFunctionDefinition($1, *$2, nullptr, $6);
		delete $2;
	}
	| type IDENTIFIER '(' VOID ')' ';' {
		$$ = new toyc::ast::NFunctionDefinition($1, *$2, nullptr, nullptr);
		delete $2;
	}
	| type IDENTIFIER '(' ')' ';' {
		$$ = new toyc::ast::NFunctionDefinition($1, *$2, nullptr, nullptr);
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
		$$ = new toyc::ast::NParameter($1, *$2);
		delete $2;
	}
	| ELLIPSIS {
		$$ = new toyc::ast::NParameter(nullptr, "", true);
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
	| if_statement {
		$$ = $1;
	}
	| for_statement {
		$$ = $1;
	}
	| while_statement {
		$$ = $1;
	}
	| do_while_statement {
		$$ = $1;
	}
	;

for_statement_init_declaration
	: expression_statement {
		$$ = $1;
	}
	| declaration_statement {
		$$ = $1;
	}
	;

for_statement
	: FOR '(' for_statement_init_declaration expression ';' expression ')' statement {
		$$ = new toyc::ast::NForStatement($3, $4, $6, $8);
	}
	;

while_statement
	: WHILE '(' expression ')' statement {
		$$ = new toyc::ast::NWhileStatement($3, $5);
	}
	;

do_while_statement
	: DO statement WHILE '(' expression ')' ';' {
		$$ = new toyc::ast::NWhileStatement($5, $2, true);
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
		$$ = new toyc::ast::NDeclarator(*$1, nullptr);
		delete $1;
	}
	| IDENTIFIER '=' expression {
		$$ = new toyc::ast::NDeclarator(*$1, $3);
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

if_statement
	: IF '(' expression ')' statement {
		$$ = new toyc::ast::NIfStatement($3, $5);
	}
	| IF '(' expression ')' statement ELSE statement {
		$$ = new toyc::ast::NIfStatement($3, $5, $7);
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

expression
	: assignment_expression {
		$$ = $1;
	  }
	;

assignment_expression
	: conditional_expression {
		$$ = $1;
	  }
	| unary_expression '=' assignment_expression {
		$$ = new toyc::ast::NAssignment($1, $3);
	  }
	| unary_expression MUL_ASSIGN assignment_expression {
		$$ = new toyc::ast::NAssignment($1, new toyc::ast::NBinaryOperator($1, toyc::ast::BineryOperator::MUL, $3));
	  }
	| unary_expression DIV_ASSIGN assignment_expression {
		$$ = new toyc::ast::NAssignment($1, new toyc::ast::NBinaryOperator($1, toyc::ast::BineryOperator::DIV, $3));
	  }
	| unary_expression ADD_ASSIGN assignment_expression {
		$$ = new toyc::ast::NAssignment($1, new toyc::ast::NBinaryOperator($1, toyc::ast::BineryOperator::ADD, $3));
	  }
	| unary_expression SUB_ASSIGN assignment_expression {
		$$ = new toyc::ast::NAssignment($1, new toyc::ast::NBinaryOperator($1, toyc::ast::BineryOperator::SUB, $3));
	  }
	| unary_expression MOD_ASSIGN assignment_expression {
		$$ = new toyc::ast::NAssignment($1, new toyc::ast::NBinaryOperator($1, toyc::ast::BineryOperator::MOD, $3));
	  }
	;

conditional_expression
	: logical_or_expression {
		$$ = $1;
	  }
	| logical_or_expression '?' expression ':' conditional_expression {
		$$ = new toyc::ast::NConditionalExpression($1, $3, $5);
	  }
	;

logical_or_expression
	: logical_and_expression {
		$$ = $1;
	  }
	| logical_or_expression OR_OP expression {
		$$ = new toyc::ast::NBinaryOperator($1, toyc::ast::BineryOperator::OR, $3);
	  }
	;

logical_and_expression
	: inclusive_or_expression {
		$$ = $1;
	  }
	| logical_and_expression  AND_OP expression {
		$$ = new toyc::ast::NBinaryOperator($1, toyc::ast::BineryOperator::AND, $3);
	  }
	;

inclusive_or_expression
	: exclusive_or_expression {
		$$ = $1;
	  }
	| inclusive_or_expression '|' exclusive_or_expression {
		$$ = new toyc::ast::NBinaryOperator($1, toyc::ast::BineryOperator::BIT_OR, $3);
	  }
	;

exclusive_or_expression
	: and_expression {
		$$ = $1;
	  }
	| exclusive_or_expression '^' and_expression {
		$$ = new toyc::ast::NBinaryOperator($1, toyc::ast::BineryOperator::XOR, $3);
	  }
	;

and_expression
	: equality_expression {
		$$ = $1;
	  }
	| and_expression '&' equality_expression {
		$$ = new toyc::ast::NBinaryOperator($1, toyc::ast::BineryOperator::BIT_AND, $3);
	  }
	;

equality_expression
	: equality_expression EQ_OP relational_expression {
		$$ = new toyc::ast::NBinaryOperator($1, toyc::ast::BineryOperator::EQ, $3);
	  }
	| equality_expression NE_OP relational_expression {
		$$ = new toyc::ast::NBinaryOperator($1, toyc::ast::BineryOperator::NE, $3);
	  }
	| relational_expression {
		$$ = $1;
	  }
	;

relational_expression
	: relational_expression relational_expression_op additive_expression {
		$$ = new toyc::ast::NBinaryOperator($1, $2, $3);
	  }
	| additive_expression {
		$$ = $1;
	  }
	;

additive_expression
	: multiplicative_expression {
		$$ = $1;
	  }
	| additive_expression '+' multiplicative_expression {
		$$ = new toyc::ast::NBinaryOperator($1, toyc::ast::BineryOperator::ADD, $3);
	  }
	| additive_expression '-' multiplicative_expression {
		$$ = new toyc::ast::NBinaryOperator($1, toyc::ast::BineryOperator::SUB, $3);
	  }
	;

multiplicative_expression
	: unary_expression {
		$$ = $1;
	  }
	| multiplicative_expression '*' unary_expression {
		$$ = new toyc::ast::NBinaryOperator($1, toyc::ast::BineryOperator::MUL, $3);
	  }
	| multiplicative_expression '/' unary_expression {
		$$ = new toyc::ast::NBinaryOperator($1, toyc::ast::BineryOperator::DIV, $3);
	  }
	| multiplicative_expression '%' unary_expression {
		$$ = new toyc::ast::NBinaryOperator($1, toyc::ast::BineryOperator::MOD, $3);
	  }
	;

unary_expression
	: postfix_expression {
		$$ = $1;
	  }
	| INC_OP unary_expression {
		$$ = new toyc::ast::NUnaryExpression(toyc::ast::UnaryOperator::L_INC, $2);
	  }
	| DEC_OP unary_expression {
		$$ = new toyc::ast::NUnaryExpression(toyc::ast::UnaryOperator::L_DEC, $2);
	  }
	| '&' unary_expression {
		$$ = new toyc::ast::NUnaryExpression(toyc::ast::UnaryOperator::ADDR, $2);
	  }
	| '*' unary_expression {
		$$ = new toyc::ast::NUnaryExpression(toyc::ast::UnaryOperator::DEREF, $2);
	  }
	| '+' unary_expression {
		$$ = new toyc::ast::NUnaryExpression(toyc::ast::UnaryOperator::PLUS, $2);
	  }
	| '-' unary_expression {
		$$ = new toyc::ast::NUnaryExpression(toyc::ast::UnaryOperator::MINUS, $2);
	  }
	| '~' unary_expression {
		$$ = new toyc::ast::NUnaryExpression(toyc::ast::UnaryOperator::BIT_NOT, $2);
	  }
	| '!' unary_expression {
		$$ = new toyc::ast::NUnaryExpression(toyc::ast::UnaryOperator::LOG_NOT, $2);
	  }
	;

postfix_expression
	: primary_expression {
		$$ = $1;
	  }
	| postfix_expression INC_OP {
		$$ = new toyc::ast::NUnaryExpression(toyc::ast::UnaryOperator::R_INC, $1);
	  }
	| postfix_expression DEC_OP {
		$$ = new toyc::ast::NUnaryExpression(toyc::ast::UnaryOperator::R_DEC, $1);
	  }
	| IDENTIFIER '(' argument_expression_list ')' {
		$$ = new toyc::ast::NFunctionCall(*$1, $3);
		delete $1;
	  }
	| IDENTIFIER '(' ')' {
		$$ = new toyc::ast::NFunctionCall(*$1, nullptr);
		delete $1;
	  }
	;

/* constant_expression
	: conditional_expression
	; */

argument_expression_list
	: assignment_expression {
		$$ = new toyc::ast::NArguments($1);
	}
	| assignment_expression ',' argument_expression_list {
		$$ = new toyc::ast::NArguments($1);
		$$->next = $3;
	}
	;

primary_expression
	: '(' expression ')' {
		$$ = $2;
	  }
	| IDENTIFIER {
		$$ = new toyc::ast::NIdentifier(*$1);
		delete $1;
	  }
    | numeric {
        $$ = $1;
      }
    | STRING_LITERAL {
        $$ = new toyc::ast::NString(*$1);
		delete $1;
      }

numeric
    : I_CONSTANT {
        $$ = new toyc::ast::NInteger(atoi($1->c_str()));
		delete $1;
      }
    | F_CONSTANT {
		$$ = new toyc::ast::NFloat(atof($1->c_str()));
		delete $1;
	  }
    ;

relational_expression_op
	: LE_OP {
		$$ = toyc::ast::BineryOperator::LE;
	}
	| GE_OP {
		$$ = toyc::ast::BineryOperator::GE;
	}
	| '<' {
		$$ = toyc::ast::BineryOperator::LT;
	}
	| '>' {
		$$ = toyc::ast::BineryOperator::GT;
	}
	;

type
	: type '*' {
		$$ = new toyc::ast::NType(toyc::ast::VarType::VAR_TYPE_PTR, $1);
	}
	| TYPEDEF_NAME {
		$$ = new toyc::ast::NType(toyc::ast::VarType::VAR_TYPE_DEFINED, *$1);
		delete $1;
	}
	| BOOL {
		$$ = new toyc::ast::NType(toyc::ast::VarType::VAR_TYPE_BOOL);
	}
	| CHAR {
		$$ = new toyc::ast::NType(toyc::ast::VarType::VAR_TYPE_CHAR);
	}
	| SHORT {
		$$ = new toyc::ast::NType(toyc::ast::VarType::VAR_TYPE_SHORT);
	}
	| INT {
		$$ = new toyc::ast::NType(toyc::ast::VarType::VAR_TYPE_INT);
	}
	| LONG {
		$$ = new toyc::ast::NType(toyc::ast::VarType::VAR_TYPE_LONG);
	}
	| FLOAT {
		$$ = new toyc::ast::NType(toyc::ast::VarType::VAR_TYPE_FLOAT);
	}
	| DOUBLE {
		$$ = new toyc::ast::NType(toyc::ast::VarType::VAR_TYPE_DOUBLE);
	}
	| VOID {
		$$ = new toyc::ast::NType(toyc::ast::VarType::VAR_TYPE_VOID);
	}
	;

%%
#include <stdio.h>

void yyerror(const char *str)
{
	if (error_handler != nullptr) {
		delete error_handler;
	}

    error_handler = new toyc::utility::ErrorHandler(std::string(str), yylineno, yycolumn - yyleng, yyleng);
}

/* void insert_symbol(const std::string &s, int type)
{
	symbol_table[s] = type;
} */