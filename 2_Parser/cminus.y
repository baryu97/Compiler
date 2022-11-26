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

%token IF THEN ELSE WHILE RETURN INT VOID 
%token ID NUM 
%token ASSIGN EQ NE LT LE GT GE PLUS MINUS TIMES OVER LPAREN RPAREN LBRACE RBRACE LCURLY RCURLY SEMI COMMA
%token ERROR 

%% /* Grammar for TINY */

program     : decl_list
                 { savedTree = $1;} 
            ;
decl_list   : decl_list decl 
              { 
                $$ = $1;
                $$->sibling = $2;
              }
            | decl { $$ = $1; }
            ;
decl        : var_decl { $$ = $1; }
            | fun_decl { $$ = $1; }
            ;
var_decl    : type_spec ID SEMI
              {
                $$ = newDeclNode(VarK);
                $$->attr.name = copyString(tokenString);
                /* type 어케 출력? */
              }
            | type_spec ID LBRACE NUM RBRACE SEMI
              {
                $$ = newDeclNode(VarK);
                $$->attr.name = copyString(tokenString);
                $$->child[0] = $4;

              }
            ;
type_spec   : INT
            | VOID
            ;
fun_decl    : type_spec ID LPAREN params RPAREN comp_stmt
              {
                $$ = newDeclNode(Funck);
                $$->type = $1;
                $$->name = copyString(tokenString);
                $$->child[0] = $4;
                $$->child[1] = $6;
              }
            ;
params      : param_list 
            | VOID          
            ;
param       : type_spec ID 
              {
                $$->type = $1;
                $$->name = copyString(tokenString);
              }
            | type_spec ID LBRACE RBRACE
              {
                $$->type = $1; /* type 어케함? */
                $$->name = copyString(tokenString);
              }
            ;            
comp_stmt   : LCURLY local_decls stmt_list RCURLY
              {
                $$->child[0] = $2;
                $$->child[1] = $3;
              }
            ;
local_decls : local_decls var_decls
              {
                $$ = $1;
                $$->sibling = $2;
              }
            | empty
              {
                $$ = NULL;
              }
            ;
stmt_list   : stmt_list stmt
              {
                $$ = $1;
                $$->sibling = $2;
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
selec_stmt  : IF LPAREN exp RPAREN stmt else stmt
              {
                $$ = newStmtNode(If_Elsek);
                $$->child[0] = $3;
                $$->child[1] = $5;
                $$->child[2] = $7;
              }
            | IF LPAREN exp RPAREN stmt
              {
                $$ = newStmtNode(Ifk);
                $$->child[0] = $3;
                $$->child[1] = $5
              }
            ;
iter_stmt   : WHILE LPAREN exp RPAREN stmt
              {
                $$ = newStmtNode(Whilek);
                $$->child[0] = $3;
                $$->child[1] = $5;
              }
            ;
ret_stmt    : RETURN SEMI
              {
                $$ = newStmtNode(Returnk);
                $$->type = Void;
              }
            | RETURN exp SEMI 
              {
                $$ = newStmtNode(Returnk);
                $$->child[0] = $2;
                $$->type = $$->child[0]->type;
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
var         : ID 
              { 
                $$ = newExpNode(VarAccK);
                $$->attr.name = copyString(tokenString); 
              }
            | ID LBRACE exp RBRACE
              {
                $$ = newExpNode(VarAccK);
                $$->attr.name = copyString(tokenString);
                $$->child[0] = $3;
              }
            ;
simple_exp  : add_exp relop add_exp
              {
                $$ = newExpNode(BiOpK);
                $$->child[0] = $1;
                $$->child[1] = $3;
                $$->attr.op = $2;
              }
            | add_exp
            ; 
relop       : LE
            | L
            | G
            | GE
            | EQ
            | NE
            ;
add_exp     : add_exp addop term 
              {
                $$ = newExpNode(BiOpK);
                $$->child[0] = $1;
                $$->child[1] = $3;
                $$->attr.op = $2; 
              }
            | term
            ;
addop       : PLUS
            | MINUS
            ;
term        : term mulop factor 
              {
                $$ = newExpNode(BiOpK);
                $$->child[0] = $1;
                $$->child[1] = $3;
                $$->attr.op = $2; 
              }
            | factor
            ;
mulop       : TIMES
            | OVER
            ;
factor      : LPAREN exp RPAREN
              {
                $$ = $2;
              }
            | var
            | call
            | NUM 
              {
                $$ = newExpNode(ConstK);
                $$->attr.val = atoi(tokenString);
              }
            ;
call        : ID LPAREN args RPAREN
              {
                $$ = newExpNode(CallK);
                $$->child[0] = $3;
                $$->attr.name = copyString(tokenString);
              }
            ;
args        : arg_list 
            | empty { $$ = NULL; }
            ;
arg_list    : arg_list COMMA exp 
              {
                $$ = $1;
                $$->sibling = $3;
              }
            | exp
            ;
            /*
stmt        : if_stmt { $$ = $1; }
            | repeat_stmt { $$ = $1; }
            | assign_stmt { $$ = $1; }
            | read_stmt { $$ = $1; }
            | write_stmt { $$ = $1; }
            | error  { $$ = NULL; }
            ;
exp         : simple_exp LT simple_exp 
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = LT;
                 }
            | simple_exp EQ simple_exp
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = EQ;
                 }
            | simple_exp { $$ = $1; }
            ;
simple_exp  : simple_exp PLUS term 
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = PLUS;
                 }
            | simple_exp MINUS term
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = MINUS;
                 } 
            | term { $$ = $1; }
            ;
term        : term TIMES factor 
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = TIMES;
                 }
            | term OVER factor
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = OVER;
                 }
            | factor { $$ = $1; }
            ;
factor      : LPAREN exp RPAREN
                 { $$ = $2; }
            | NUM
                 { $$ = newExpNode(ConstK);
                   $$->attr.val = atoi(tokenString);
                 }
            | ID { $$ = newExpNode(IdK);
                   $$->attr.name =
                         copyString(tokenString);
                 }
            | error { $$ = NULL; }
            ;
            */


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

