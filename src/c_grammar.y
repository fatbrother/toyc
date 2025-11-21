%{
#include <string>
#include <iostream>

#include "ast/node.hpp"
#include "ast/expression.hpp"
#include "ast/statement.hpp"
#include "ast/external_definition.hpp"
#include "utility/error_handler.hpp"
#include "semantic/parser_actions.hpp"

void yyerror(const char *s);
extern "C" int yylex(void);
extern int yyparse();

extern int yylineno;
extern int yycolumn;
extern char* yytext;
extern int yyleng;

// Global parser state
toyc::ast::NExternalDeclaration *program = nullptr;
toyc::utility::ErrorHandler *error_handler = nullptr;
toyc::semantic::ParserActions *parser_actions = nullptr;

%}

%union
{
	toyc::ast::NExpression *expression;
	toyc::ast::TypeDescriptor *type_specifier;
	toyc::ast::NDeclarator *declarator;
	toyc::ast::NStatement *statement;
	toyc::ast::NDeclarationStatement *declaration_specifiers;
	toyc::ast::NBlock *block;
	toyc::ast::NExternalDeclaration *external_declaration;
	toyc::ast::NParameter *parameter;
	toyc::ast::NArguments *arguments;
	toyc::ast::NStructDeclaration *struct_declaration;
	toyc::ast::NInitializerList *initializer_list;
	toyc::ast::BineryOperator bop;
	std::string *string;
	int token;
	int integer;
}

%define parse.error verbose

%token	<string> IDENTIFIER I_CONSTANT F_CONSTANT C_CONSTANT STRING_LITERAL TYPEDEF_NAME
%token	INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP PTR_OP AND_OP OR_OP
%token	MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token	SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token	XOR_ASSIGN OR_ASSIGN

%token	TYPEDEF SIZEOF
%token	BOOL CHAR SHORT INT LONG FLOAT DOUBLE VOID
%token	SIGNED UNSIGNED CONST STATIC
%token	STRUCT UNION ENUM ELLIPSIS

%token	CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%type	<expression> expression unary_expression assignment_expression relational_expression equality_expression shift_expression additive_expression
%type	<expression> multiplicative_expression primary_expression numeric postfix_expression conditional_expression logical_or_expression constant_expression
%type	<expression> logical_and_expression inclusive_or_expression exclusive_or_expression and_expression initializer cast_expression
%type   <initializer_list> initializer_list
%type   <type_specifier> type_specifier struct_specifier type_name
%type   <struct_declaration> struct_declaration struct_declaration_list
%type	<bop> relational_expression_op
%type   <declarator> init_declarator init_declarator_list struct_declarator_list declarator direct_declarator
%type   <parameter> parameter_list parameter_declaration
%type   <declaration_specifiers> declaration_specifiers
%type   <statement> statement statement_list expression_statement jump_statement for_statement_init_declaration
%type   <statement>  if_statement for_statement while_statement do_while_statement labeled_statement switch_statement
%type   <block> compound_statement
%type   <external_declaration> program external_declaration external_declaration_list function_definition
%type   <arguments> argument_expression_list
%type   <integer> pointer

%start program
%%

program
	: external_declaration_list {
		program = $1;
	}
	;

external_declaration_list
	: external_declaration external_declaration_list {
		$$ = parser_actions->handleExternalDeclarationList($1, $2);
	}
	| external_declaration {
		$$ = $1;
	}
	;

external_declaration
	: function_definition {
		$$ = $1;
	}
	| declaration_specifiers {
		$$ = $1;
	}
	;

function_definition
	:  type_specifier IDENTIFIER '(' parameter_list ')' compound_statement {
		$$ = parser_actions->handleFunctionDefinition($1, *$2, $4, $6, @$.first_line, @$.first_column);
		delete $2;
	}
	|  type_specifier IDENTIFIER '(' ')' compound_statement {
		$$ = parser_actions->handleFunctionDefinition($1, *$2, nullptr, $5, @$.first_line, @$.first_column);
		delete $2;
	}
	|  type_specifier IDENTIFIER '(' parameter_list ')' ';' {
		$$ = parser_actions->handleFunctionDeclaration($1, *$2, $4, @$.first_line, @$.first_column);
		delete $2;
	}
	|  type_specifier IDENTIFIER '(' VOID ')' compound_statement {
		$$ = parser_actions->handleFunctionDefinition($1, *$2, nullptr, $6, @$.first_line, @$.first_column);
		delete $2;
	}
	|  type_specifier IDENTIFIER '(' VOID ')' ';' {
		$$ = parser_actions->handleFunctionDeclaration($1, *$2, nullptr, @$.first_line, @$.first_column);
		delete $2;
	}
	|  type_specifier IDENTIFIER '(' ')' ';' {
		$$ = parser_actions->handleFunctionDeclaration($1, *$2, nullptr, @$.first_line, @$.first_column);
		delete $2;
	}
	;

parameter_list
	: parameter_declaration ',' parameter_list {
		$$ = parser_actions->handleParameterList($1, $3, @$.first_line, @$.first_column);
	}
	| parameter_declaration {
		$$ = $1;
	}
	;

parameter_declaration
	:  type_specifier declarator {
		$$ = parser_actions->handleParameter($1, $2, @$.first_line, @$.first_column);
	}
	| ELLIPSIS {
		$$ = parser_actions->handleVariadicParameter();
	}
	;

compound_statement
	: '{' statement_list '}' {
		$$ = parser_actions->handleCompoundStatement($2);
	}
	| '{' '}' {
		$$ = parser_actions->handleEmptyCompoundStatement();
	}
	;

statement_list
	: statement statement_list {
		$$ = parser_actions->handleStatementList($1, $2);
	}
	| statement {
		$$ = $1;
	}
	;

statement
	: expression_statement {
		$$ = $1;
	}
	| declaration_specifiers {
		$$ = $1;
	}
	| compound_statement {
		$$ = $1;
	}
	| jump_statement {
		$$ = $1;
	}
	| labeled_statement {
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
	| switch_statement {
		$$ = $1;
	}
	;

for_statement_init_declaration
	: expression_statement {
		$$ = $1;
	}
	| declaration_specifiers {
		$$ = $1;
	}
	;

for_statement
	: FOR '(' for_statement_init_declaration expression ';' expression ')' statement {
		$$ = parser_actions->handleForStatement($3, $4, $6, $8);
	}
	;

while_statement
	: WHILE '(' expression ')' statement {
		$$ = parser_actions->handleWhileStatement($3, $5);
	}
	;

do_while_statement
	: DO statement WHILE '(' expression ')' ';' {
		$$ = parser_actions->handleDoWhileStatement($5, $2);
	}
	;

switch_statement
	: SWITCH '(' expression ')' compound_statement {
		$$ = parser_actions->handleSwitchStatement($3, $5);
	}
	;

labeled_statement
	: IDENTIFIER ':' statement {
		$$ = parser_actions->handleLabelStatement(*$1, $3);
		delete $1;
	}
	| CASE expression ':' {
		$$ = parser_actions->handleCaseStatement($2);
	}
	| DEFAULT ':' {
		$$ = parser_actions->handleDefaultStatement();
	}
	;

jump_statement
	: GOTO IDENTIFIER ';' {
		$$ = parser_actions->handleGotoStatement(*$2);
		delete $2;
	}
	| RETURN expression ';' {
		$$ = parser_actions->handleReturnStatement($2);
	}
	| RETURN ';' {
		$$ = parser_actions->handleReturnStatement();
	}
	| BREAK ';' {
		$$ = parser_actions->handleBreakStatement();
	}
	| CONTINUE ';' {
		$$ = parser_actions->handleContinueStatement();
	}
	;

declaration_specifiers
	:  type_specifier init_declarator_list ';' {
		$$ = parser_actions->handleDeclarationStatement($1, $2);
	}
	| type_specifier ';' {
		$$ = parser_actions->handleEmptyDeclaration($1);
	}
	;

init_declarator_list
	: init_declarator {
		$$ = $1;
	}
	| init_declarator ',' init_declarator_list {
		$$ = $1;
		$$->next = $3;
	}
	;

init_declarator
	: declarator {
		$$ = $1;
	}
	| declarator '=' initializer {
		$$ = parser_actions->handleInitDeclarator($1, $3);
	}
	;

declarator
	: pointer direct_declarator {
		$$ = parser_actions->handleDeclarator($1, $2);
	}
	| direct_declarator {
		$$ = $1;
	}
	;

direct_declarator
	: IDENTIFIER {
		$$ = parser_actions->handleDeclarator($1);
	}
	| direct_declarator '[' constant_expression ']' {
		$$ = parser_actions->handleArrayDeclarator($1, $3);
	}
	| direct_declarator '[' ']' {
		$$ = parser_actions->handleArrayDeclarator($1);
	}
	;

pointer
	: '*' {
		$$ = 1;
	}
	| '*' pointer {
		$$ = $2 + 1;
	}
	;

initializer
	: assignment_expression {
		$$ = $1;
	}
	| '{' initializer_list '}' {
		$$ = $2;
	}
	| '{' initializer_list ',' '}' {
		$$ = $2;
	}
	;

initializer_list
	: initializer {
		$$ = parser_actions->handleInitializerList($1, nullptr);
	}
	| initializer_list ',' initializer {
		$$ = parser_actions->handleInitializerList($3, $1);
	}
	;

if_statement
	: IF '(' expression ')' statement {
		$$ = parser_actions->handleIfStatement($3, $5);
	}
	| IF '(' expression ')' statement ELSE statement {
		$$ = parser_actions->handleIfStatement($3, $5, $7);
	}
	;

expression_statement
	: expression ';' {
		$$ = parser_actions->handleExpressionStatement($1);
	}
	| ';' {
		$$ = parser_actions->handleEmptyExpressionStatement();
	}
	;

expression
	: assignment_expression {
		$$ = $1;
	  }
	| expression ',' assignment_expression {
		$$ = parser_actions->handleCommaExpression($1, $3);
	  }
	;

assignment_expression
	: conditional_expression {
		$$ = $1;
	  }
	| unary_expression '=' assignment_expression {
		$$ = parser_actions->handleAssignment($1, $3);
	  }
	| unary_expression MUL_ASSIGN assignment_expression {
		$$ = parser_actions->handleCompoundAssignment($1, toyc::ast::BineryOperator::MUL, $3);
	  }
	| unary_expression DIV_ASSIGN assignment_expression {
		$$ = parser_actions->handleCompoundAssignment($1, toyc::ast::BineryOperator::DIV, $3);
	  }
	| unary_expression ADD_ASSIGN assignment_expression {
		$$ = parser_actions->handleCompoundAssignment($1, toyc::ast::BineryOperator::ADD, $3);
	  }
	| unary_expression SUB_ASSIGN assignment_expression {
		$$ = parser_actions->handleCompoundAssignment($1, toyc::ast::BineryOperator::SUB, $3);
	  }
	| unary_expression MOD_ASSIGN assignment_expression {
		$$ = parser_actions->handleCompoundAssignment($1, toyc::ast::BineryOperator::MOD, $3);
	  }
	| unary_expression AND_ASSIGN assignment_expression {
		$$ = parser_actions->handleCompoundAssignment($1, toyc::ast::BineryOperator::BIT_AND, $3);
	  }
	| unary_expression OR_ASSIGN assignment_expression {
		$$ = parser_actions->handleCompoundAssignment($1, toyc::ast::BineryOperator::BIT_OR, $3);
	  }
	| unary_expression XOR_ASSIGN assignment_expression {
		$$ = parser_actions->handleCompoundAssignment($1, toyc::ast::BineryOperator::XOR, $3);
	  }
	| unary_expression LEFT_ASSIGN assignment_expression {
		$$ = parser_actions->handleCompoundAssignment($1, toyc::ast::BineryOperator::LEFT, $3);
	  }
	| unary_expression RIGHT_ASSIGN assignment_expression {
		$$ = parser_actions->handleCompoundAssignment($1, toyc::ast::BineryOperator::RIGHT, $3);
	  }
	;

conditional_expression
	: logical_or_expression {
		$$ = $1;
	  }
	| logical_or_expression '?' expression ':' conditional_expression {
		$$ = parser_actions->handleConditionalExpression($1, $3, $5);
	  }
	;

logical_or_expression
	: logical_and_expression {
		$$ = $1;
	  }
	| logical_or_expression OR_OP expression {
		$$ = parser_actions->handleBinaryExpression(toyc::ast::BineryOperator::OR, $1, $3);
	  }
	;

logical_and_expression
	: inclusive_or_expression {
		$$ = $1;
	  }
	| logical_and_expression  AND_OP expression {
		$$ = parser_actions->handleBinaryExpression(toyc::ast::BineryOperator::AND, $1, $3);
	  }
	;

inclusive_or_expression
	: exclusive_or_expression {
		$$ = $1;
	  }
	| inclusive_or_expression '|' exclusive_or_expression {
		$$ = parser_actions->handleBinaryExpression(toyc::ast::BineryOperator::BIT_OR, $1, $3);
	  }
	;

exclusive_or_expression
	: and_expression {
		$$ = $1;
	  }
	| exclusive_or_expression '^' and_expression {
		$$ = parser_actions->handleBinaryExpression(toyc::ast::BineryOperator::XOR, $1, $3);
	  }
	;

and_expression
	: equality_expression {
		$$ = $1;
	  }
	| and_expression '&' equality_expression {
		$$ = parser_actions->handleBinaryExpression(toyc::ast::BineryOperator::BIT_AND, $1, $3);
	  }
	;

equality_expression
	: equality_expression EQ_OP relational_expression {
		$$ = parser_actions->handleBinaryExpression(toyc::ast::BineryOperator::EQ, $1, $3);
	  }
	| equality_expression NE_OP relational_expression {
		$$ = parser_actions->handleBinaryExpression(toyc::ast::BineryOperator::NE, $1, $3);
	  }
	| relational_expression {
		$$ = $1;
	  }
	;

relational_expression
	: relational_expression relational_expression_op shift_expression {
		$$ = parser_actions->handleBinaryExpression($2, $1, $3);
	  }
	| shift_expression {
		$$ = $1;
	  }
	;

shift_expression
	: additive_expression {
		$$ = $1;
	  }
	| shift_expression LEFT_OP additive_expression {
		$$ = parser_actions->handleBinaryExpression(toyc::ast::BineryOperator::LEFT, $1, $3);
	  }
	| shift_expression RIGHT_OP additive_expression {
		$$ = parser_actions->handleBinaryExpression(toyc::ast::BineryOperator::RIGHT, $1, $3);
	  }
	;

additive_expression
	: multiplicative_expression {
		$$ = $1;
	  }
	| additive_expression '+' multiplicative_expression {
		$$ = parser_actions->handleBinaryExpression(toyc::ast::BineryOperator::ADD, $1, $3);
	  }
	| additive_expression '-' multiplicative_expression {
		$$ = parser_actions->handleBinaryExpression(toyc::ast::BineryOperator::SUB, $1, $3);
	  }
	;

multiplicative_expression
	: cast_expression {
		$$ = $1;
	  }
	| multiplicative_expression '*' cast_expression {
		$$ = parser_actions->handleBinaryExpression(toyc::ast::BineryOperator::MUL, $1, $3);
	  }
	| multiplicative_expression '/' cast_expression {
		$$ = parser_actions->handleBinaryExpression(toyc::ast::BineryOperator::DIV, $1, $3);
	  }
	| multiplicative_expression '%' cast_expression {
		$$ = parser_actions->handleBinaryExpression(toyc::ast::BineryOperator::MOD, $1, $3);
	  }
	;

cast_expression
	: unary_expression {
		$$ = $1;
	  }
	| '(' type_specifier pointer ')' cast_expression {
		$$ = parser_actions->handleCastExpressionWithPointer($2, $3, $5);
	  }
	| '(' type_specifier ')' cast_expression {
		$$ = parser_actions->handleCastExpression($2, $4);
	  }
	;

unary_expression
	: postfix_expression {
		$$ = $1;
	  }
	| INC_OP unary_expression {
		$$ = parser_actions->handleUnaryExpression(toyc::ast::UnaryOperator::L_INC, $2);
	  }
	| DEC_OP unary_expression {
		$$ = parser_actions->handleUnaryExpression(toyc::ast::UnaryOperator::L_DEC, $2);
	  }
	| '&' cast_expression {
		$$ = parser_actions->handleUnaryExpression(toyc::ast::UnaryOperator::ADDR, $2);
	  }
	| '*' cast_expression {
		$$ = parser_actions->handleUnaryExpression(toyc::ast::UnaryOperator::DEREF, $2);
	  }
	| '+' cast_expression {
		$$ = parser_actions->handleUnaryExpression(toyc::ast::UnaryOperator::PLUS, $2);
	  }
	| '-' cast_expression {
		$$ = parser_actions->handleUnaryExpression(toyc::ast::UnaryOperator::MINUS, $2);
	  }
	| '~' cast_expression {
		$$ = parser_actions->handleUnaryExpression(toyc::ast::UnaryOperator::BIT_NOT, $2);
	  }
	| '!' cast_expression {
		$$ = parser_actions->handleUnaryExpression(toyc::ast::UnaryOperator::LOG_NOT, $2);
	  }
	| SIZEOF unary_expression {
		$$ = parser_actions->handleSizeofExpression($2);
	  }
	| SIZEOF '(' type_name ')' {
		$$ = parser_actions->handleSizeofType($3);
	  }
	;

postfix_expression
	: primary_expression {
		$$ = $1;
	  }
	| postfix_expression '[' expression ']' {
		$$ = parser_actions->handleArrayAccess($1, $3);
	  }
	| postfix_expression INC_OP {
		$$ = parser_actions->handleUnaryExpression(toyc::ast::UnaryOperator::R_INC, $1);
	  }
	| postfix_expression DEC_OP {
		$$ = parser_actions->handleUnaryExpression(toyc::ast::UnaryOperator::R_DEC, $1);
	  }
	| IDENTIFIER '(' argument_expression_list ')' {
		$$ = parser_actions->handleFunctionCall(*$1, $3);
		delete $1;
	  }
	| IDENTIFIER '(' ')' {
		$$ = parser_actions->handleFunctionCall(*$1, nullptr);
		delete $1;
	  }
	| postfix_expression '.' IDENTIFIER {
		$$ = parser_actions->handleMemberAccess($1, *$3, false);
		delete $3;
	  }
	| postfix_expression PTR_OP IDENTIFIER {
		$$ = parser_actions->handleMemberAccess($1, *$3, true);
		delete $3;
	  }
	;

constant_expression
	: conditional_expression {
		$$ = $1;
	}
	;

argument_expression_list
	: assignment_expression {
		$$ = parser_actions->handleArgumentList($1, nullptr);
	}
	| assignment_expression ',' argument_expression_list {
		$$ = parser_actions->handleArgumentList($1, $3);
	}
	;

primary_expression
	: '(' expression ')' {
		$$ = $2;
	  }
	| IDENTIFIER {
		$$ = parser_actions->handleIdentifier(*$1);
		delete $1;
	  }
    | numeric {
        $$ = $1;
      }
    | STRING_LITERAL {
        $$ = parser_actions->handleString(*$1);
		delete $1;
      }

numeric
    : I_CONSTANT {
        $$ = parser_actions->handleIntegerFromString(*$1);
		delete $1;
      }
	| C_CONSTANT {
		$$ = parser_actions->handleCharConstant(*$1);
		delete $1;
	  }
    | F_CONSTANT {
		$$ = parser_actions->handleFloat(*$1);
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

type_specifier
	: TYPEDEF_NAME {
		$$ = nullptr;
	}
	| BOOL {
		$$ = parser_actions->handlePrimitiveType("bool");
	}
	| CHAR {
		$$ = parser_actions->handlePrimitiveType("char");
	}
	| SHORT {
		$$ = parser_actions->handlePrimitiveType("short");
	}
	| INT {
		$$ = parser_actions->handlePrimitiveType("int");
	}
	| LONG {
		$$ = parser_actions->handlePrimitiveType("long");
	}
	| FLOAT {
		$$ = parser_actions->handlePrimitiveType("float");
	}
	| DOUBLE {
		$$ = parser_actions->handlePrimitiveType("double");
	}
	| VOID {
		$$ = parser_actions->handlePrimitiveType("void");
	}
	| struct_specifier {
		$$ = $1;
	}
	;

struct_specifier
	: STRUCT IDENTIFIER '{' struct_declaration_list '}' {
		$$ = parser_actions->handleStructSpecifier(*$2, $4);
		delete $2;
	}
	| STRUCT '{' struct_declaration_list '}' {
		$$ = parser_actions->handleAnonymousStruct($3);
	}
	| STRUCT IDENTIFIER {
		$$ = parser_actions->handleStructReference(*$2);
		delete $2;
	}
	;

type_name
	: type_specifier {
		$$ = $1;
	}
	| type_specifier pointer {
		$$ = parser_actions->handleTypeNameWithPointer($1, $2);
	}
	| type_specifier '[' I_CONSTANT ']' {
		$$ = parser_actions->handleTypeNameWithArray($1, *$3);
		delete $3;
	}
	| type_specifier pointer '[' I_CONSTANT ']' {
		$$ = parser_actions->handleTypeNameWithPointerAndArray($1, $2, *$4);
		delete $4;
	}
	;

struct_declaration_list
	: struct_declaration {
		$$ = $1;
	}
	| struct_declaration struct_declaration_list {
		$$ = $1;
		$$->next = $2;
	}
	| /* empty */ {
		$$ = nullptr;
	}
	;

struct_declaration
	:  type_specifier struct_declarator_list ';' {
		$$ = parser_actions->handleStructDeclaration($1, $2);
	}
	;

struct_declarator_list
	: declarator {
		$$ = $1;
	}
	| struct_declarator_list ',' declarator {
		$$ = $1;
		$$->next = $3;
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