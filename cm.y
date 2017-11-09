/****************************************************/
/* File: cm.y                                       */
/* The TINY Yacc/Bison specification file           */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/* Modified for C-                                  */
/* CSC 425: Compilers and Interpreters              */
/* James Graci, Logan Stecker                       */
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
static int savedNumber;
static TreeNode * savedTree; /* stores syntax tree for later return */
static int yylex(void); /* added 11/2/11 to ensure no conflict with lex */

int yyerror(char * message); /* prototype to make gcc happy */

%}

/*Reserved words*/
%token ELSE IF INT RETURN VOID WHILE

/*Multichar Tokens*/
%token ID NUM 

/*Special Symbols as defined by Appendix A*/
%token PLUS MINUS DIVIDE MULT SMALLER ESMALLER LARGER ELARGER EQUAL NOTEQUAL ASSIGN SEMI COMMA LPARENT RPARENT LBRACKET RBRACKET LBPARENT RBPARENT
     
%token ERROR 

%% /* Grammar for C- */
program     : decl_list
                 { savedTree = $1;}
            ;
decl_list   : decl_list decl
                 { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; }
                     else $$ = $2;
                 }
            | decl  { $$ = $1; }
            ;
decl        : var_decl    { $$ = $1; }
            | fun_decl    { $$ = $1; }
	    | param_decl  { $$ = $1; }
            ;
saveName    : ID
                 { savedName = copyString(tokenString);
                   savedLineNo = lineno;
                 }
            ;
saveNumber  : NUM
                 { savedNumber = atol(tokenString);
                   savedLineNo = lineno;
                 }
            ;
var_decl    : type_spec saveName SEMI
                 { $$ = newDeclNode(VarK);
                   $$->child[0] = $1; /* type */
                   $$->lineno = lineno;
                   $$->attr.name = savedName;
                 }
            | type_spec saveName LBPARENT saveNumber RBPARENT SEMI
                 { $$ = newDeclNode(VarK);
                   $$->child[0] = $1; /* type */
                   $$->lineno = lineno;
		   $$->type = Array;
                   $$->attr.name = savedName;
                 }
            ;
fun_decl    : type_spec saveName {
                   $$ = newDeclNode(FunK);
                   $$->lineno = lineno;
                   $$->attr.name = savedName;
                 }
              LPARENT params RPARENT comp_stmt
                 {
                   $$ = $3;
                   $$->child[0] = $1; /* type */
                   $$->child[1] = $5;    /* parameters */
                   $$->child[2] = $7; /* body */
                 }
            ;
param_decl  : param_decl COMMA param
                 { YYSTYPE t = $1;
		   if (t != NULL)
		   { while (t->sibling != NULL)
		        t = t->sibling;
                     t->sibling = $3;
		     $$ = $1; }
		     else $$ = $3;
		 }
              | param { $$ = $1; };
type_spec   : INT   {}
            | VOID  {}
            ;
params      : param_decl  { $$ = $1; }
            | VOID
                 { $$ = newDeclNode(ParamK);
                   $$->type = Void;
                 }
param       : type_spec saveName
                 { $$ = newDeclNode(ParamK);
		   $$->child[0] = $1;
		   $$->attr.name = savedName;
		 }
            | type_spec saveName LBPARENT RBPARENT
                 { $$ = newDeclNode(ParamK);
                   $$->child[0] = $1;
		   $$->type = Array;
                   $$->attr.name = savedName;
                 }
            ;
comp_stmt   : LBRACKET local_decls stmt_list RBRACKET
                 { $$ = newStmtNode(CompK);
                   $$->child[0] = $2;
                   $$->child[1] = $3;
                 }
            ;
local_decls : local_decls var_decl
                 { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; }
                     else $$ = $2;
                 }
            | /* empty */ { $$ = NULL; }
            ;
stmt_list   : stmt_list stmt
                 { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $2;
                     $$ = $1; }
                     else $$ = $2;
                 }
            | /* empty */ { $$ = NULL; }
            ;
stmt        : exp_stmt { $$ = $1; }
            | comp_stmt { $$ = $1; }
            | sel_stmt { $$ = $1; }
            | iter_stmt { $$ = $1; }
            | ret_stmt { $$ = $1; }
            ;
exp_stmt    : exp SEMI { $$ = $1; }
            | SEMI { $$ = NULL; }
            ;
sel_stmt    : IF LPARENT exp RPARENT stmt
                 { $$ = newStmtNode(SelK);
                   $$->child[0] = $3;
                   $$->child[1] = $5;
                   $$->child[2] = NULL;
                 }
            | IF LPARENT exp RPARENT stmt ELSE stmt
                 { $$ = newStmtNode(SelK);
                   $$->child[0] = $3;
                   $$->child[1] = $5;
                   $$->child[2] = $7;
                 }
            ;
iter_stmt   : WHILE LPARENT exp RPARENT stmt
                 { $$ = newStmtNode(IterK);
                   $$->child[0] = $3;
                   $$->child[1] = $5;
                 }
            ;
ret_stmt    : RETURN SEMI
                 { $$ = newStmtNode(RetK);
                   $$->child[0] = NULL;
                 }
            | RETURN exp SEMI
                 { $$ = newStmtNode(RetK);
                   $$->child[0] = $2;
                 }
            ;
exp         : var ASSIGN exp
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
		   $$->attr.op = ASSIGN;
                 }
            | simple_exp { $$ = $1; }
            ;
var         : saveName
                 { $$ = newExpNode(IdK);
                   $$->attr.name = savedName;
                 }
            | saveName LBPARENT exp RBPARENT
                 { $$ = newExpNode(IdK);
                   $$->attr.name = savedName;
		   $$->child[0] = $1;
		   $$->child[1] = $3;
		   $$->type = Array;
		 }
            ;
simple_exp  : add_exp ESMALLER add_exp
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = ESMALLER;
                 }
            | add_exp SMALLER add_exp
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = SMALLER;
                 }
            | add_exp LARGER add_exp
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = LARGER;
                 }
            | add_exp ELARGER add_exp
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = ELARGER;
                 }
            | add_exp EQUAL add_exp
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = EQUAL;
                 }
            | add_exp NOTEQUAL add_exp
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = NOTEQUAL;
                 }
            | add_exp { $$ = $1; }
            ;
add_exp     : add_exp PLUS term
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = PLUS;
                 }
            | add_exp MINUS term
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = MINUS;
                 }
            | term { $$ = $1; }
            ;
term        : term MULT factor
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = MULT;
                 }
            | term DIVIDE factor
                 { $$ = newExpNode(OpK);
                   $$->child[0] = $1;
                   $$->child[1] = $3;
                   $$->attr.op = DIVIDE;
                 }
            | factor { $$ = $1; }
            ;
factor      : LPARENT exp RPARENT { $$ = $2; }
            | var { $$ = $1; }
            | call { $$ = $1; }
            | NUM
                 { $$ = newExpNode(ConstK);
                   $$->attr.val = atoi(tokenString);
                 }
            ;
call        : saveName {
                 $$ = newExpNode(CallK);
                 $$->attr.name = savedName;
              }
              LPARENT args RPARENT
                 { $$ = $2;
                   $$->child[0] = $4;
                 }
            ;
args        : arg_list { $$ = $1; }
            | /* empty */ { $$ = NULL; }
            ;
arg_list    : arg_list COMMA exp
                 { YYSTYPE t = $1;
                   if (t != NULL)
                   { while (t->sibling != NULL)
                        t = t->sibling;
                     t->sibling = $3;
                     $$ = $1; }
                     else $$ = $3;
                 }
            | exp { $$ = $1; }
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

