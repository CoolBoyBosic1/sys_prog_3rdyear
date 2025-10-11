%{
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <memory>
#include <string>
/* !!! тут НЕ оголошуємо yylex і НЕ підключаємо ast.hpp, щоб не плутати заголовок */
%}

/* Це потрапить у parser.tab.h — там потрібно знати, що таке AST */
%code requires { #include "ast.hpp" }

/* Це піде у parser.tab.c — тут уже можна оголосити змінні/ф-ції */
%code {
std::unique_ptr<Program> g_program;
void yyerror(const char* s);
int yylex(void); 
}

%define parse.error verbose

%union {
    int    ival;
    double dval;
    char*  sval;
    AST*   node;
    std::vector<AST*>* vec;
}

/* типізовані токени */
%token T_INT T_DOUBLE T_BOOL T_TRUE T_FALSE
%token T_IF T_ELSE T_WHILE T_FOR T_RETURN
%token <sval>  T_IDENT
%token <dval>  T_NUMBER_D
%token T_EQ T_NE T_LE T_GE T_AND T_OR

/* нетермінали */
%type <node> program external decl type opt_init func_def param_list_opt param_list param
%type <node> stmt stmt_list_opt compound expr opt_expr arg_list_opt arg_list

%left T_OR
%left T_AND
%left T_EQ T_NE
%left '<' '>' T_LE T_GE
%left '+' '-'
%left '*' '/' '%'
%right '!' '='

%%
program
  : external                 { g_program = std::make_unique<Program>(); g_program->items.push_back($1); }
  | program external         { g_program->items.push_back($2); }
  ;

external
  : decl ';'                 { $$ = $1; }
  | func_def                 { $$ = $1; }
  ;

type
  : T_INT                    { $$ = new TypeNode("int"); }
  | T_DOUBLE                 { $$ = new TypeNode("double"); }
  | T_BOOL                   { $$ = new TypeNode("bool"); }
  ;

decl
  : type T_IDENT opt_init    { $$ = new DeclNode(as<TypeNode>($1)->name, std::string($2), as<ExprNode>($3)); free($2); }
  ;

opt_init
  : /* empty */              { $$ = nullptr; }
  | '=' expr                 { $$ = $2; }
  ;

param_list_opt
  : /* empty */              { $$ = nullptr; }
  | param_list               { $$ = $1; }
  ;

param_list
  : param                    { $$ = new ParamListNode(); as<ParamListNode>($$)->params.push_back(as<ParamNode>($1)); }
  | param_list ',' param     { $$ = $1; as<ParamListNode>($$)->params.push_back(as<ParamNode>($3)); }
  ;

param
  : type T_IDENT             { $$ = new ParamNode(as<TypeNode>($1)->name, std::string($2)); free($2); }
  ;

func_def
  : type T_IDENT '(' param_list_opt ')' compound
                            { $$ = new FuncDefNode(as<TypeNode>($1)->name, std::string($2), as<ParamListNode>($4), as<BlockNode>($6)); free($2); }
  ;

compound
  : '{' stmt_list_opt '}'    { $$ = new BlockNode(as<VecNode>($2)); }
  ;

stmt_list_opt
  : /* empty */              { $$ = new VecNode(); }
  | stmt_list_opt stmt       { as<VecNode>($1)->items.push_back($2); $$ = $1; }
  ;

stmt
  : decl ';'                 { $$ = $1; }
  | expr ';'                 { $$ = new ExprStmtNode(as<ExprNode>($1)); }
  | T_RETURN expr ';'        { $$ = new ReturnNode(as<ExprNode>($2)); }
  | T_IF '(' expr ')' stmt   { $$ = new IfNode(as<ExprNode>($3), as<Node>($5), nullptr); }
  | T_IF '(' expr ')' stmt T_ELSE stmt
                            { $$ = new IfNode(as<ExprNode>($3), as<Node>($5), as<Node>($7)); }
  | T_WHILE '(' expr ')' stmt
                            { $$ = new WhileNode(as<ExprNode>($3), as<Node>($5)); }
  | T_FOR '(' opt_expr ';' opt_expr ';' opt_expr ')' stmt
                            { $$ = new ForNode(as<ExprNode>($3), as<ExprNode>($5), as<ExprNode>($7), as<Node>($9)); }
  | compound                 { $$ = $1; }
  ;

opt_expr
  : /* empty */              { $$ = nullptr; }
  | expr                     { $$ = $1; }
  ;

expr
  : T_IDENT '=' expr         { $$ = new AssignNode(std::string($1), as<ExprNode>($3)); free($1); }
  | expr T_OR expr           { $$ = new BinOpNode("||", as<ExprNode>($1), as<ExprNode>($3)); }
  | expr T_AND expr          { $$ = new BinOpNode("&&", as<ExprNode>($1), as<ExprNode>($3)); }
  | expr T_EQ expr           { $$ = new BinOpNode("==", as<ExprNode>($1), as<ExprNode>($3)); }
  | expr T_NE expr           { $$ = new BinOpNode("!=", as<ExprNode>($1), as<ExprNode>($3)); }
  | expr '<' expr            { $$ = new BinOpNode("<", as<ExprNode>($1), as<ExprNode>($3)); }
  | expr '>' expr            { $$ = new BinOpNode(">", as<ExprNode>($1), as<ExprNode>($3)); }
  | expr T_LE expr           { $$ = new BinOpNode("<=", as<ExprNode>($1), as<ExprNode>($3)); }
  | expr T_GE expr           { $$ = new BinOpNode(">=", as<ExprNode>($1), as<ExprNode>($3)); }
  | expr '+' expr            { $$ = new BinOpNode("+", as<ExprNode>($1), as<ExprNode>($3)); }
  | expr '-' expr            { $$ = new BinOpNode("-", as<ExprNode>($1), as<ExprNode>($3)); }
  | expr '*' expr            { $$ = new BinOpNode("*", as<ExprNode>($1), as<ExprNode>($3)); }
  | expr '/' expr            { $$ = new BinOpNode("/", as<ExprNode>($1), as<ExprNode>($3)); }
  | expr '%' expr            { $$ = new BinOpNode("%", as<ExprNode>($1), as<ExprNode>($3)); }
  | '!' expr                 { $$ = new UnaryOpNode("!", as<ExprNode>($2)); }
  | '-' expr %prec '!'       { $$ = new UnaryOpNode("neg", as<ExprNode>($2)); }
  | '(' expr ')'             { $$ = $2; }
  | T_IDENT '(' arg_list_opt ')' { $$ = new CallNode(std::string($1), as<ArgListNode>($3)); free($1); }
  | T_IDENT                  { $$ = new VarRefNode(std::string($1)); free($1); }
  | T_NUMBER_D               { $$ = new NumberNode($1); }
  | T_TRUE                   { $$ = new BoolNode(true); }
  | T_FALSE                  { $$ = new BoolNode(false); }
  ;

arg_list_opt
  : /* empty */              { $$ = nullptr; }
  | arg_list                 { $$ = $1; }
  ;

arg_list
  : expr                     { $$ = new ArgListNode(); as<ArgListNode>($$)->args.push_back(as<ExprNode>($1)); }
  | arg_list ',' expr        { $$ = $1; as<ArgListNode>($$)->args.push_back(as<ExprNode>($3)); }
  ;

%%

void yyerror(const char* s){
    std::fprintf(stderr, "Parse error: %s\n", s);
}
