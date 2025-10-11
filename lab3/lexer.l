%{
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "parser.tab.h"
%}

%option noyywrap

DIGIT   [0-9]
ID      [a-zA-Z_][a-zA-Z0-9_]*
WS      [ \t\r\f\v\n]+

%%

"//"[^\n]*           ;
"/*"([^*]|\*+[^*/])*"*/" ;

{WS}                 ;

"int"                { return T_INT; }
"double"             { return T_DOUBLE; }
"bool"               { return T_BOOL; }
"true"               { yylval.ival = 1; return T_TRUE; }
"false"              { yylval.ival = 0; return T_FALSE; }
"if"                 { return T_IF; }
"else"               { return T_ELSE; }
"while"              { return T_WHILE; }
"for"                { return T_FOR; }
"return"             { return T_RETURN; }

{ID}                 { yylval.sval = strdup(yytext); return T_IDENT; }

({DIGIT}+\.({DIGIT})*|{DIGIT}*\.({DIGIT})+)([eE][+-]?{DIGIT}+)? {
                        yylval.dval = atof(yytext); return T_NUMBER_D;
                      }
{DIGIT}+([eE][+-]?{DIGIT}+)? { yylval.dval = atof(yytext); return T_NUMBER_D; }

"=="                { return T_EQ; }
"!="                { return T_NE; }
"<="                { return T_LE; }
">="                { return T_GE; }
"&&"                { return T_AND; }
"||"                { return T_OR; }
"="                 { return '='; }
"<"                 { return '<'; }
">"                 { return '>'; }
"+"                 { return '+'; }
"-"                 { return '-'; }
"*"                 { return '*'; }
"/"                 { return '/'; }
"%"                 { return '%'; }
"!"                 { return '!'; }
"("                 { return '('; }
")"                 { return ')'; }
"{"                 { return '{'; }
"}"                 { return '}'; }
","                 { return ','; }
";"                 { return ';'; }

.                   { return yytext[0]; }

%%
