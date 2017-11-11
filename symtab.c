/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* Modified for C-                                  */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "symtab.h"

#define MAX_SCOPE 1000

/* SHIFT is the power of two used as multiplier
   in hash function  */
#define SHIFT 4

/* the hash function */
static int hash ( char * key )
{ int temp = 0;
  int i = 0;
  while (key[i] != '\0')
  { temp = ((temp << SHIFT) + key[i]) % SIZE;
    ++i;
  }
  return temp;
}

static Scope scopeList[MAX_SCOPE];
static int nscope = 0;
static Scope scopeStack[MAX_SCOPE];
static int nstack = 0;
static int scope_loc[MAX_SCOPE];

/* Function scope_top
 * returns last scope level
 */
Scope scope_top( void )
{ return scopeStack[nstack - 1];
}

/* Function scope_pop
 * moves scope counter
 * to previous scope
 */
void scope_pop( void )
{ --nstack;
}

/* Function add_loc claims space
 * for current scope level
 */
int add_loc( void )
{ return scope_loc[nstack - 1]++;
}

/* Function scope_push
 * adds new scope to scope list
 */
void scope_push( Scope scope )
{ scopeStack[nstack] = scope;
  scope_loc[nstack++] = 0;
}

/* Function scope_create
 * initializes new scope
 */
Scope scope_create( char * fun_name )
{ Scope newScope;
  newScope = (Scope) malloc(sizeof(struct ScopeRec));
  newScope->fun_name = fun_name;
  newScope->nestedLevel = nstack;
  newScope->parent = scope_top();
  scopeList[nscope++] = newScope;
  return newScope;
}

/* Function st_bucket
 * checks scope for given entry
 */
BucketList st_bucket( char * name )
{ int h = hash(name);
  Scope scope = scope_top();
  while(scope)
  { BucketList l = scope->hashTable[h];
    while((l != NULL) && (strcmp(name,l->name) != 0))
      l = l->next;
    if (l != NULL)
      return l;
    scope = scope->parent;
  }
  return NULL;
}

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( char * name, int lineno, int loc, TreeNode * treeNode )
{ int h = hash(name);
  Scope top = scope_top();
  BucketList l =  top->hashTable[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) /* variable not yet in table */
  { l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->treeNode = treeNode;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;
    l->lines->next = NULL;
    l->next = top->hashTable[h];
    top->hashTable[h] = l; }
  /*
  else
  { LineList t = l->lines;
    while (t->next != NULL) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
  */
} /* st_insert */

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
int st_lookup( char * name )
{ BucketList l = st_bucket(name);
  if (l != NULL)
    return l->memloc;
  return -1;
}

/* Function st_lookup_top
 * returns location of entry
 * in current scope
 */
int st_lookup_top( char * name )
{ int h = hash(name);
  Scope scope = scope_top();
  while(scope)
  { BucketList l = scope->hashTable[h];
    while((l != NULL) && (strcmp(name,l->name) != 0))
      l = l->next;
    if(l != NULL)
      return l->memloc;
    break;
  }
  return -1;
}

/* Function st_add_lineno
 * creates line record for
 * table entry
 */
int st_add_lineno( char * name, int lineno )
{ BucketList l = st_bucket(name);
  LineList ll = l->lines;
  while(ll->next != NULL)
    ll = ll->next;
  ll->next = (LineList) malloc(sizeof(struct LineListRec));
  ll->next->lineno = lineno;
  ll->next->next = NULL;
}

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing)
{ int i;
  for (i=0;i<nscope;++i)
  { Scope scope = scopeList[i];
    BucketList * hashTable = scope->hashTable;
    fprintf(listing,"Scope Level : %d\n",scope->nestedLevel);
    fprintf(listing,"Variable Name\tType\tLine Numbers\n");
    fprintf(listing,"-------------\t----\t------------\n");
    for (int j=0;j<SIZE;++j)
    { if (hashTable[j] != NULL)
      { BucketList l = hashTable[j];
        TreeNode * node = l->treeNode;
        while(l != NULL)
        { LineList t = l->lines;
          fprintf(listing,"%-14s ",l->name);
          switch (node->type)
          { case Void:
              fprintf(listing,"Void\t");
              break;
            case Integer:
              fprintf(listing,"Integer\t");
              break;
            case Array:
              fprintf(listing,"Array\t");
              break;
            default:
              break;
          }
          while (t != NULL)
          { fprintf(listing,"%4d ",t->lineno);
            t = t->next;
          }
          fprintf(listing,"\n");
          l = l->next;
        }
      }
    }
    fputc('\n',listing);
  }
} /* printSymTab */
