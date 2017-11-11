/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* Modified for C-                                  */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"

#define SIZE 211

typedef struct LineListRec
   { int lineno;
     struct LineListRec * next;
   } * LineList;

typedef struct BucketListRec
   { char * name;
     LineList lines;
     TreeNode *treeNode;
     int memloc;
     struct BucketListRec * next;
   } * BucketList;

typedef struct ScopeRec
   { char * fun_name;
     int nestedLevel;
     struct ScopeRec * parent;
     BucketList hashTable[SIZE];
   } * Scope;

Scope globalScope;

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, int lineno, int loc, TreeNode * treeNode );

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
int st_lookup ( char * name );
int st_add_lineno ( char * name, int lineno );
BucketList st_bucket ( char * name );
int st_lookup_top ( char * name );

Scope scope_create (char * name );
Scope scope_top ( void );
void scope_pop ( void );
void scope_push ( Scope scope );
int add_loc ( void );

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing);

#endif
