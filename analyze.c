/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Modified for C-                                  */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "util.h"

static char * fun_name;
static int preserve_scope = FALSE;

static void symbolError(TreeNode * t, char * message)
{ fprintf(listing,"Symbol error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

static void insertIO(void)
{ TreeNode * fun;
  TreeNode * param;
  TreeNode * compStmt;

  fun = newDeclNode(FunK);
  fun->type = Integer;

  compStmt = newStmtNode(CompK);
  compStmt->child[0] = NULL;
  compStmt->child[1] = NULL;

  fun->lineno = 0;
  fun->attr.name = "input";
  fun->child[0] = NULL;
  fun->child[1] = compStmt;

  st_insert("input",-1,add_loc(),fun);

  fun = newDeclNode(FunK);
  fun->type = Void;

  param = newDeclNode(ParamK);
  param->attr.name = "arg";
  param->child[0] = newExpNode(IdK);
  param->child[0]->type = Integer;

  compStmt = newStmtNode(CompK);
  compStmt->child[0] = NULL;
  compStmt->child[1] = NULL;

  fun->lineno = 0;
  fun->attr.name = "output";
  fun->child[0] = param;
  fun->child[1] = compStmt;

  st_insert("output",-1,add_loc(),fun);
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t)
{ switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case CompK:
          if (preserve_scope)
            preserve_scope = FALSE;
          else
          { Scope scope = scope_create(fun_name);
            scope_push(scope);
          }
          t->attr.scope = scope_top();
          break;
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { case IdK:
        case CallK:
          if (st_lookup(t->attr.name) == -1)
          /* not yet in table, so treat as error */
            symbolError(t,"undeclared symbol");
          else
          /* already in table, so ignore location, 
             add line number of use only */ 
            st_add_lineno(t->attr.name,t->lineno);
          break;
        default:
          break;
      }
      break;
    case DeclK:
      switch (t->kind.decl)
      { case FunK:
          fun_name = t->attr.name;
          if (st_lookup_top(fun_name) >= 0)
          /* already in table, so treat as error */
          { symbolError(t,"function already declared");
            break;
          }
          st_insert(fun_name,t->lineno,add_loc(),t);
          scope_push(scope_create(fun_name));
          preserve_scope = TRUE;
          switch (t->child[0]->type)
          { case Integer:
              t->type = Integer;
              break;
            case Void:
            default:
              t->type = Void;
              break;
          }
          break;
        case VarK:
          { char * name;
            if (t->child[0]->type == Void)
            { symbolError(t,"type should not be void");
              break;
            }
            name = t->attr.name;
            t->type = Integer;
            if (st_lookup_top(name) < 0)
              st_insert(name,t->lineno,add_loc(),t);
            else
              symbolError(t,"symbol already declared in current scope");
          }
          break;
        case ParamK:
          { if (t->child[0]->type == Void)
              symbolError(t->child[0],"invalid parameter type");
            if (st_lookup(t->attr.name) == -1)
            { st_insert(t->attr.name,t->lineno,add_loc(),t);
              t->type = Integer;
            }
          }
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

static void afterInsertNode(TreeNode * t)
{ switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case CompK:
          scope_pop();
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ globalScope = scope_create(NULL);
  scope_push(globalScope);
  insertIO();
  traverse(syntaxTree,insertNode,afterInsertNode);
  scope_pop();
  if (TraceAnalyze)
  { fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
}

static void beforeCheckNode(TreeNode * t)
{ switch (t->nodekind)
  { case DeclK:
      switch (t->kind.decl)
      { case FunK:
          fun_name = t->attr.name;
          break;
        default:
          break;
      }
      break;
    case StmtK:
      switch (t->kind.stmt)
      { case CompK:
          scope_push(t->attr.scope);
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { case CompK:
          scope_pop();
          break;
        case IterK:
          if (t->child[0]->type == Void)
            typeError(t->child[0],"while test should not have void value");
          break;
        case RetK:
          { TreeNode * funDecl = st_bucket(fun_name)->treeNode;
            TypeSpec funType = funDecl->type;
            TreeNode * expr = t->child[0];

            if (funType == Void && (expr != NULL && expr->type != Void))
              typeError(t,"unexpected return value");
            else if (funType == Integer && (expr == NULL || expr->type == Void))
              typeError(t,"expected return value");
          }
          break;
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { case OpK:
          { TypeSpec type1, type2;
            TokenType op;
            type1 = t->child[0]->type;
            type2 = t->child[1]->type;
            op = t->attr.op;
            if (type1 == Void || type2 == Void)
              typeError(t,"operands must not have void type");
            else if (type1 == Array && type2 == Array)
              typeError(t,"operands must not both be arrays");
            else if (op == MINUS && type1 == Integer && type2 == Array)
              typeError(t,"invalid operands");
            else if ((op == MULT || op == DIVIDE) && (type1 == Array || type2 == Array))
              typeError(t,"invalid operands");
            else
              t->type = Integer;
          }
          break;
        case ConstK:
          t->type = Integer;
          break;
        case IdK:
          { char * id_name = t->attr.name;
            BucketList bucket = st_bucket(id_name);
            TreeNode * id_decl = NULL;
            if (bucket == NULL)
              break;
            id_decl = bucket->treeNode;
            if (t->type == Array)
            { if (id_decl->type != Array)
                typeError(t,"expected array");
              else if (t->child[0]->type != Integer)
                typeError(t,"indexed expression must be of type integer");
              else
                t->type = Integer;
            }
            else
              t->type = id_decl->type;
          }
          break;
        case CallK:
          { char * call_name = t->attr.name;
            TreeNode * fun_decl = st_bucket(call_name)->treeNode;
            TreeNode * arg;
            TreeNode * param;
            if (fun_decl == NULL)
              break;
            arg = t->child[0];
            param = fun_decl->child[1];
            if (fun_decl->kind.decl != FunK)
            { typeError(t,"expected function");
              break;
            }
            while (arg != NULL)
            { if (param != NULL)
                typeError(arg,"wrong number of parameters");
              else if (arg->type == Void)
                typeError(arg,"cannot pass void value");
              else
              { arg = arg->sibling;
                param = param->sibling;
                continue;
              }
              break;
            }
            if (arg == NULL && param != NULL)
              typeError(t->child[0],"wrong number of parameters");
            t->type = fun_decl->type;
          }
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ scope_push(globalScope);
  traverse(syntaxTree,beforeCheckNode,checkNode);
  scope_pop();
}
