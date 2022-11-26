/****************************************************/
/* File: tiny.y                                     */
/* The TINY Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/
%{
#define YYPARSER /* distinguishes Yacc output from other code files */

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

#define YYSTYPE TreeNode *
static char * savedName; /* for use in assignments */
static int savedLineNo;  /* ditto */
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void); // added 11/2/11 to ensure no conflict with lex

%}

%token ELSE IF THEN WHILE RETURN INT VOID 
%token ID NUM 
%token ASSIGN EQ NE LT LE GT GE PLUS MINUS TIMES OVER LPAREN RPAREN LBRACE RBRACE LCURLY RCURLY SEMI COMMA
%token ERROR 

%% /* Grammar for TINY */

program     : decl_list
                 { savedTree = $1;} 
            ;
decl_list   : decl_list decl 
              { 
                YYSTYPE t = $1;
                if (t != NULL)
                { 
                  while (t->sibling != NULL)
                    t = t->sibling;
                  t->sibling = $2;
                  $$ = $1; 
                }
                else $$ = $2;
              }
            | decl { $$ = $1; }
            ;
decl        : var_decl { $$ = $1; }
            | fun_decl { $$ = $1; }
            ;
var_decl    : type_spec id SEMI 
              {
                $$ = newDeclNode(VarK);
                $$->type = $1->type;
                free($1);
                $$->attr.name = $2->attr.name;
                free($2);
              }
            | arr_spec id LBRACE num RBRACE SEMI
              {
                $$ = newDeclNode(VarK);
                $$->type = $1->type;
                free($1);
                $$->attr.name = $2->attr.name;
                free($2);
                $$->child[0] = $4;
              }
            ;
num         : NUM 
              {
                $$ = newExpNode(ConstK);
                $$->attr.val = atoi(tokenString);
              }
            ;
id          : ID
              { 
                $$ = newDeclNode(VarK);
                savedName = copyString(tokenString);
                $$->attr.name = savedName;
              }
            ;
type_spec   : INT
              {
                $$ = newDeclNode(VarK);
                $$->type = Integer;
              }
            | VOID
              {
                $$ = newDeclNode(VarK);
                $$->type = Void;
              }
            ;
arr_spec    : INT
              {
                $$ = newDeclNode(VarK);
                $$->type = IntArr;
              }
            | VOID
              {
                $$ = newDeclNode(VarK);
                $$->type = VoidArr;
              }
            ;
fun_decl    : type_spec id LPAREN params RPAREN comp_stmt
              {
                $$ = newDeclNode(FuncK);
                $$->type = $1->type;
                free($1);
                $$->attr.name = $2->attr.name;
                $$->child[0] = $4;
                $$->child[1] = $6;
              }
            ;
params      : param_list 
            | VOID 
              {
                $$ = newDeclNode(ParaK);
                $$->type = Void;
              }         
            ;
param_list  : param_list COMMA param
              { 
                YYSTYPE t = $1;
                if (t != NULL)
                { 
                  while (t->sibling != NULL)
                    t = t->sibling;
                  t->sibling = $3;
                  $$ = $1; 
                }
                else $$ = $3;
              }
            | param
            ;
param       : type_spec id 
              {
                $$ = newDeclNode(ParaK);
                $$->type = $1->type;
                free($1);
                $$->attr.name = $2->attr.name;
                free($2);
              }
            | arr_spec id LBRACE RBRACE
              {
                $$ = newDeclNode(ParaK);
                $$->type = $1->type;
                free($1);
                $$->attr.name = $2->attr.name;
                free($2);
              }
            ;            
comp_stmt   : LCURLY local_decls stmt_list RCURLY
              {
                $$ = newStmtNode(CompK);
                $$->child[0] = $2;
                $$->child[1] = $3;
              }
            ;
local_decls : local_decls var_decl
              { 
                YYSTYPE t = $1;
                if (t != NULL)
                { 
                  while (t->sibling != NULL)
                    t = t->sibling;
                  t->sibling = $2;
                  $$ = $1; 
                }
                else $$ = $2;
              }
            | /* empty */
              {
                $$ = NULL;
              }
            ;
stmt_list   : stmt_list stmt
              { 
                YYSTYPE t = $1;
                if (t != NULL)
                { 
                  while (t->sibling != NULL)
                    t = t->sibling;
                  t->sibling = $2;
                  $$ = $1; 
                }
                else $$ = $2;
              }
            | stmt
            ;
stmt        : exp_stmt
            | comp_stmt
            | selec_stmt
            | iter_stmt
            | ret_stmt
            ;
exp_stmt    : exp SEMI { $$ = $1; }
            | SEMI { $$ = NULL; }
            ;
selec_stmt  : %prec ELSE
              IF LPAREN exp RPAREN stmt ELSE stmt 
              {
                $$ = newStmtNode(If_ElseK);
                $$->child[0] = $3;
                $$->child[1] = $5;
                $$->child[2] = $7;
              }
            | IF LPAREN exp RPAREN stmt 
              {
                $$ = newStmtNode(IfK);
                $$->child[0] = $3;
                $$->child[1] = $5;
              }
            ;
iter_stmt   : WHILE LPAREN exp RPAREN stmt
              {
                $$ = newStmtNode(WhileK);
                $$->child[0] = $3;
                $$->child[1] = $5;
              }
            ;
ret_stmt    : RETURN SEMI
              {
                $$ = newStmtNode(ReturnK);
                $$->type = Void;
              }
            | RETURN exp SEMI 
              {
                $$ = newStmtNode(ReturnK);
                $$->child[0] = $2;
                $$->type = Integer;
              }
            ;
exp         : var ASSIGN exp
              {
                $$ = newExpNode(AssignK);
                $$->child[0] = $1;
                $$->child[1] = $3;
              }
            | simple_exp
            ;
var         : id 
              { 
                $$ = newExpNode(VarAccK);
                $$->attr.name = $1->attr.name;
                free($1);
              }
            | id LBRACE exp RBRACE
              {
                $$ = newExpNode(VarAccK);
                $$->attr.name = $1->attr.name;
                free($1);
                $$->child[0] = $3;
              }
            ;
simple_exp  : add_exp LT add_exp
              {
                $$ = newExpNode(BiOpK);
                $$->child[0] = $1;
                $$->child[1] = $3;
                $$->attr.op = LT;
              }
            | add_exp LE add_exp
              {
                $$ = newExpNode(BiOpK);
                $$->child[0] = $1;
                $$->child[1] = $3;
                $$->attr.op = LE;
              }
            | add_exp GT add_exp
              {
                $$ = newExpNode(BiOpK);
                $$->child[0] = $1;
                $$->child[1] = $3;
                $$->attr.op = GT;
              }
            | add_exp GE add_exp
              {
                $$ = newExpNode(BiOpK);
                $$->child[0] = $1;
                $$->child[1] = $3;
                $$->attr.op = GE;
              }
            | add_exp EQ add_exp
              {
                $$ = newExpNode(BiOpK);
                $$->child[0] = $1;
                $$->child[1] = $3;
                $$->attr.op = EQ;
              }
            | add_exp NE add_exp
              {
                $$ = newExpNode(BiOpK);
                $$->child[0] = $1;
                $$->child[1] = $3;
                $$->attr.op = NE;
              }
            | add_exp
            ; 
/*
relop       : LE
            | LT
            | GT
            | GE
            | EQ
            | NE
            ;
*/
add_exp     : add_exp PLUS term 
              {
                $$ = newExpNode(BiOpK);
                $$->child[0] = $1;
                $$->child[1] = $3;
                $$->attr.op = PLUS; 
              }
            | add_exp MINUS term 
              {
                $$ = newExpNode(BiOpK);
                $$->child[0] = $1;
                $$->child[1] = $3;
                $$->attr.op = MINUS; 
              }
            | term
            ;
/*
addop       : PLUS
            | MINUS
            ;
*/
term        : term TIMES factor 
              {
                $$ = newExpNode(BiOpK);
                $$->child[0] = $1;
                $$->child[1] = $3;
                $$->attr.op = TIMES; 
              }
            | term OVER factor 
              {
                $$ = newExpNode(BiOpK);
                $$->child[0] = $1;
                $$->child[1] = $3;
                $$->attr.op = OVER; 
              }
            | factor
            ;
/* 
mulop       : TIMES
            | OVER
            ;
            */
factor      : LPAREN exp RPAREN
              {
                $$ = $2;
              }
            | var
            | call
            | num
            ;
call        : id LPAREN args RPAREN
              {
                $$ = newExpNode(CallK);
                $$->child[0] = $3;
                $$->attr.name = $1->attr.name;
                free($1);
              }
            ;
args        : arg_list 
            | /* empty */ { $$ = NULL; }
            ;
arg_list    : arg_list COMMA exp 
              { 
                YYSTYPE t = $1;
                if (t != NULL)
                { 
                  while (t->sibling != NULL)
                    t = t->sibling;
                  t->sibling = $3;
                  $$ = $1; 
                }
                else $$ = $3;
              }
            | exp
            ;


%%

int yyerror(char * message)
{ fprintf(listing,"Syntax error at line %d: %s\n",lineno,message);
  fprintf(listing,"Current token: ");
  printToken(yychar,tokenString);
  Error = TRUE;
  return 0;
}

/* yylex calls getToken to make Yacc/Bison output
 * compatible with ealier versions of the TINY scanner
 */
static int yylex(void)
{ return getToken(); }

TreeNode * parse(void)
{ yyparse();
  return savedTree;
}

