/****************************************************/
/* File: cm.l                                       */
/* Lex specification for TINY                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/* Modified for C-                                  */
/* CSC 425: Compilers and Interpreters              */
/* James Graci, Logan Stecker                       */
/****************************************************/

%{
#include "globals.h"
#include "util.h"
#include "scan.h"
/* lexeme of identifier or reserved word */
char tokenString[MAXTOKENLEN+1];
%}

digit       [0-9]
number      {digit}+
letter      [a-zA-Z]
identifier  {letter}+
newline     \n
whitespace  [ \t]+

%%

"if" {return IF;}
"else" {return ELSE;}
"int" {return INT;}
"return" {return RETURN;}
"void" {return VOID;}
"while" {return WHILE;}
"+" {return PLUS;}
"-" {return MINUS;}
"*" {return MULT;}
"/" {return DIVIDE;}
"<" {return SMALLER;}
"<=" {return ESMALLER;}
">" {return LARGER;}
">=" {return ELARGER;}
"==" {return EQUAL;}
"!=" {return NOTEQUAL;}
"=" {return ASSIGN;}
";" {return SEMI;}
"," {return COMMA;}
"(" {return LPARENT;}
")" {return RPARENT;}
"[" {return LBPARENT;}
"]" {return RBPARENT;}
"{" {return LBRACKET;}
"}" {return RBRACKET;}
{number} {return NUM;}
{identifier} {return ID;}
{newline} {lineno++;}
{whitespace} { /* skip whitespace */ }
"/*" { char c;
       char d;
       do {
        c = input();
        if (c == EOF) break;
        if (c == '\n') lineno++;
        if (c == '*') /* to close the comment, '/' must follow '*' */ {
          d = input();
          if (d == '/') break;
        }
       } while (c);
     }
  "//" { char s; /* skip input until next line */
         do {
           s = input();
           if (s == EOF) break;
           if (s == '\n') /* must increment line as well as close comment */ {
             lineno++;
             break;
           }
         } while (s);
       }
. { return ERROR; }

%%

TokenType getToken(void)
{ static int firstTime = TRUE;
  TokenType currentToken;
  if (firstTime)
  { firstTime = FALSE;
    lineno++;
    yyin = source;
    yyout = listing;
  }
  currentToken = yylex();
  strncpy(tokenString,yytext,MAXTOKENLEN);
  if (TraceScan) {
    fprintf(listing,"\t%d: ",lineno);
    printToken(currentToken,tokenString);
  }
  return currentToken;
}

