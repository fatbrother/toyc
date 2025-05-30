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

#include <iostream>
#include "ast/node.hpp"
#include "ast/expression.hpp"
#include "ast/statement.hpp"
#include "ast/external_definition.hpp"
#include "y.tab.hpp"

// extern std::unordered_map<std::string, std::string> symbol_table;
extern int yylineno;
extern char* yytext;
extern "C" int yylex(void);
int yycolumn = 1;

extern void yyerror(const char *);  /* prints grammar violation message */
static int check_type(void);
static char* unescape_string(const char* yytext, int yyleng);

#define SAVE_TOKEN yylval.string = new char[yyleng + 1]; \
strncpy(yylval.string, yytext, yyleng); \
yylval.string[yyleng] = '\0'; \
yycolumn += yyleng;

#define SAVE_STRING yylval.string = unescape_string(yytext, yyleng); yycolumn += yyleng;
#define TOKEN(t) (yylval.token = t); return t; yycolumn += yyleng;

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

{L}{A}*					{ SAVE_TOKEN; return check_type(); }

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

({SP}?\"([^"\\\n]|{ES})*\"{WS}*)+	{ SAVE_STRING; return STRING_LITERAL; }

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
"<"					    { yycolumn += yyleng; return '<'; }
">"					    { yycolumn += yyleng; return '>'; }
"-"					    { yycolumn += yyleng; return '-'; }
"+"					    { yycolumn += yyleng; return '+'; }
"*"					    { yycolumn += yyleng; return '*'; }
"/"					    { yycolumn += yyleng; return '/'; }
"%"					    { yycolumn += yyleng; return '%'; }
"^"					    { yycolumn += yyleng; return '^'; }
"&"					    { yycolumn += yyleng; return '&'; }
"|"					    { yycolumn += yyleng; return '|'; }
";"					    { yycolumn += yyleng; return ';'; }
("{"|"<%")				{ yycolumn += yyleng; return '{'; }
("}"|"%>")				{ yycolumn += yyleng; return '}'; }
","					    { yycolumn += yyleng; return ','; }
":"					    { yycolumn += yyleng; return ':'; }
"="					    { yycolumn += yyleng; return '='; }
"("					    { yycolumn += yyleng; return '('; }
")"					    { yycolumn += yyleng; return ')'; }
("["|"<:")				{ yycolumn += yyleng; return '['; }
("]"|":>")				{ yycolumn += yyleng; return ']'; }
"."					    { yycolumn += yyleng; return '.'; }
"!"					    { yycolumn += yyleng; return '!'; }
"~"					    { yycolumn += yyleng; return '~'; }
"?"					    { yycolumn += yyleng; return '?'; }

[ \t\r]+                { yycolumn += yyleng; }
\n                      { yycolumn = 1; }

.					    { yyterminate();}

%%

static int check_type(void)
{
    return IDENTIFIER;
}

static char* unescape_string(const char* yytext, int yyleng) {
    char* result = (char*)malloc(yyleng);
    char* dst = result;
    const char* src = yytext + 1;

    while (*src && *(src) != '"') {
        if (*src == '\\') {
            src++;
            switch (*src) {
                case 'n': *dst++ = '\n'; break;
                case 't': *dst++ = '\t'; break;
                case 'r': *dst++ = '\r'; break;
                case '\\': *dst++ = '\\'; break;
                case '"': *dst++ = '"'; break;
                default: *dst++ = *src; break;
            }
        } else {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';

    return result;
}