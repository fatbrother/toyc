O   [0-7]
D   [0-9]
NZ  [1-9]
L   [a-zA-Z_]
A   [a-zA-Z_0-9]
H   [a-fA-F0-9]
HP  (0[xX])
E   ([Ee][+-]?{D}+)
P   ([Pp][+-]?{D}+)
FS  (f|F|l|L)
IS  (((u|U)(l|L|ll|LL)?)|((l|L|ll|LL)(u|U)?))
CP  (u|U|L)
SP  (u8|u|U|L)
ES  (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
WS  [ \t\v\n\f]

%{
#include <stdio.h>
#include <string>
#include <unordered_map>

struct ValueType {
    std::string string;
    int integer;
    double real;
};

extern std::unordered_map<std::string, int> symbol_table;
extern int yylineno;
extern char* yytext;
extern "C" int yylex(void);
int yycolumn = 1;

#define YYSTYPE ValueType
#include "parser/y.tab.hpp"

extern void yyerror(const char *);  /* prints grammar violation message */
extern int sym_type(const char *);  /* returns type of symbol */

#define SAVE_TOKEN yylval.string = std::string(yytext); yycolumn += yyleng;
#define TOKEN(t) yycolumn += yyleng; return t;
#define YYTEXT_STR std::string(yytext)

static int check_type(void);
%}

%x COMMENT
%x MULTI_COMMENT

%option noyywrap
%option yylineno

%%

\/\/            { BEGIN COMMENT; }
<COMMENT>[^\n]* { ; }
<COMMENT>\n     { BEGIN 0; }

\/\*            { BEGIN MULTI_COMMENT; }
<MULTI_COMMENT>[^*]* { ; }
<MULTI_COMMENT>\*+[^*/]* { ; }
<MULTI_COMMENT>\*+\/ { BEGIN 0; }

"break"					{ TOKEN(BREAK); }
"bool"                  { TOKEN(BOOL); }
"case"					{ TOKEN(CASE); }
"char"					{ TOKEN(CHAR); }
"const"					{ TOKEN(CONST); }
"continue"				{ TOKEN(CONTINUE); }
"default"				{ TOKEN(DEFAULT); }
"do"					{ TOKEN(DO); }
"double"				{ TOKEN(DOUBLE); }
"else"					{ TOKEN(ELSE); }
"float"					{ TOKEN(FLOAT); }
"for"					{ TOKEN(FOR); }
"goto"					{ TOKEN(GOTO); }
"if"					{ TOKEN(IF); }
"int"					{ TOKEN(INT); }
"long"					{ TOKEN(LONG); }
"return"				{ TOKEN(RETURN); }
"short"					{ TOKEN(SHORT); }
"signed"				{ TOKEN(SIGNED); }
"sizeof"				{ TOKEN(SIZEOF); }
"static"				{ TOKEN(STATIC); }
"struct"				{ TOKEN(STRUCT); }
"switch"				{ TOKEN(SWITCH); }
"typedef"				{ TOKEN(TYPEDEF); }
"unsigned"				{ TOKEN(UNSIGNED); }
"void"					{ TOKEN(VOID); }
"while"					{ TOKEN(WHILE); }

{L}{A}*					{ return check_type(); }

{HP}{H}+{IS}?				        { SAVE_TOKEN; return I_CONSTANT; }
{NZ}{D}*{IS}?				        { SAVE_TOKEN; return I_CONSTANT; }
"0"{O}*{IS}?				        { SAVE_TOKEN; return I_CONSTANT; }
{CP}?"'"([^'\\\n]|{ES})+"'"		    { SAVE_TOKEN; return I_CONSTANT; }

{D}+{E}{FS}?				        { SAVE_TOKEN; return F_CONSTANT; }
{D}*"."{D}+{E}?{FS}?			    { SAVE_TOKEN; return F_CONSTANT; }
{D}+"."{E}?{FS}?			        { SAVE_TOKEN; return F_CONSTANT; }
{HP}{H}+{P}{FS}?			        { SAVE_TOKEN; return F_CONSTANT; }
{HP}{H}*"."{H}+{P}{FS}?			    { SAVE_TOKEN; return F_CONSTANT; }
{HP}{H}+"."{P}{FS}?			        { SAVE_TOKEN; return F_CONSTANT; }

({SP}?\"([^"\\\n]|{ES})*\"{WS}*)+	{ SAVE_TOKEN; return STRING_LITERAL; }

"..."					{ TOKEN(ELLIPSIS); }
">>="					{ TOKEN(RIGHT_ASSIGN); }
"<<="					{ TOKEN(LEFT_ASSIGN); }
"+="					{ TOKEN(ADD_ASSIGN); }
"-="					{ TOKEN(SUB_ASSIGN); }
"*="					{ TOKEN(MUL_ASSIGN); }
"/="					{ TOKEN(DIV_ASSIGN); }
"%="					{ TOKEN(MOD_ASSIGN); }
"&="					{ TOKEN(AND_ASSIGN); }
"^="					{ TOKEN(XOR_ASSIGN); }
"|="					{ TOKEN(OR_ASSIGN); }
">>"					{ TOKEN(RIGHT_OP); }
"<<"					{ TOKEN(LEFT_OP); }
"++"					{ TOKEN(INC_OP); }
"--"					{ TOKEN(DEC_OP); }
"->"					{ TOKEN(PTR_OP); }
"&&"					{ TOKEN(AND_OP); }
"||"					{ TOKEN(OR_OP); }
"<="					{ TOKEN(LE_OP); }
">="					{ TOKEN(GE_OP); }
"=="					{ TOKEN(EQ_OP); }
"!="					{ TOKEN(NE_OP); }
";"					    { return ';'; }
("{"|"<%")				{ return '{'; }
("}"|"%>")				{ return '}'; }
","					    { return ','; }
":"					    { return ':'; }
"="					    { return '='; }
"("					    { return '('; }
")"					    { return ')'; }
("["|"<:")				{ return '['; }
("]"|":>")				{ return ']'; }
"."					    { return '.'; }
"&"					    { return '&'; }
"!"					    { return '!'; }
"~"					    { return '~'; }
"-"					    { return '-'; }
"+"					    { return '+'; }
"*"					    { return '*'; }
"/"					    { return '/'; }
"%"					    { return '%'; }
"<"					    { return '<'; }
">"					    { return '>'; }
"^"					    { return '^'; }
"|"					    { return '|'; }
"?"					    { return '?'; }

[ \t\r]+        { yycolumn += yyleng; }
\n              { yycolumn = 1; }

.					{ /* discard bad characters */ }

%%

static int check_type(void)
{
    if (symbol_table.find(YYTEXT_STR) != symbol_table.end()) {
        yylval.integer = symbol_table[YYTEXT_STR];
        return IDENTIFIER;
    }
    return TYPEDEF_NAME;
}