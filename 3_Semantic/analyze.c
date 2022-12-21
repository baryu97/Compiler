/****************************************************/
/* File: analyze.c                                  */
/* Semantic analyzer implementation                 */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "symtab.h"
#include "analyze.h"
#include "string.h"

#define MAX_DEPTH 1024

/* counter for variable memory locations */
static int location = 0;

ScopeList scope_stack[MAX_DEPTH];
int sp = 0;
int rtsp = 0;
ExpKind ret_type_stack[MAX_DEPTH];
ScopeList global;

void stack_push(ScopeList s){
  scope_stack[sp++] = s;
}

void rt_push(ExpKind s){
  ret_type_stack[rtsp++] = s;
}

void stack_pop(){
  if (sp)
    sp--;
}

void rt_pop(){
  if (rtsp)
    rtsp--;
}

ScopeList stack_top(){
  if (sp == 0)
    return NULL;
  return scope_stack[sp - 1];
}

ExpKind rt_top(){
  if(rtsp == 0)
    return 1;
  return ret_type_stack[rtsp - 1];
}

int func_comp = 0;

int i = 0;

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine:
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse( TreeNode * t,
               void (* preProc) (TreeNode *),
               void (* postProc) (TreeNode *) )
{ 
  if (t != NULL)
  { preProc(t);
    { int i;
      for (i=0; i < MAXCHILDREN; i++)
        traverse(t->child[i],preProc,postProc);
    }
    // printf("%d\n",++i);
    postProc(t);
    traverse(t->sibling,preProc,postProc);
  }
}

/* nullProc is a do-nothing procedure to 
 * generate preorder-only or postorder-only
 * traversals from traverse
 */
static void nullProc(TreeNode * t)
{ if (t==NULL) return;
  else return;
}

void pop_scope(TreeNode *t){
  if (t->nodekind == StmtK && t->kind.stmt == CompK)
  {
    // printf("aaaaaaaaa");
    // printf("p%s%d\n",stack_top()->name,t->lineno);
    stack_pop();
  }
}

/* Procedure insertNode inserts 
 * identifiers stored in t into 
 * the symbol table 
 */
static void insertNode( TreeNode * t)
{ 
  ScopeList cur_scope = stack_top();
  switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { 
        case CompK :
          if (func_comp){
            func_comp = 0;
          } 
          else {
            char *name = (char *) malloc(100);
            snprintf(name, 100, "%s: %d", cur_scope->name, t->lineno);
            ScopeList s = insert_scope(name);
            t->scope = s;
            s->parent = cur_scope;
            if (cur_scope->child == NULL)
              cur_scope->child = s;
            else {
              ScopeList k = cur_scope->child;
              while (k->next_scope != NULL)
              {
                k = k->next_scope;
              }
              k->next_scope = s;
            }
            stack_push(s);
          }
        // case AssignK:
        // case ReadK:
        //   if (st_lookup(t->attr.name) == -1)
        //   /* not yet in table, so treat as new definition */
        //     st_insert(t->attr.name,t->lineno,location++);
        //   else
        //   /* already in table, so ignore location, 
        //      add line number of use only */ 
        //     st_insert(t->attr.name,t->lineno,0);
        //   break;
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { 
        // case IdK:
        //   if (st_lookup(t->attr.name) == -1)
        //   /* not yet in table, so treat as new definition */
        //     st_insert(t->attr.name,t->lineno,location++);
        //   else
        //   /* already in table, so ignore location, 
        //      add line number of use only */ 
        //     st_insert(t->attr.name,t->lineno,0);
        //   break;
        default:
          break;
      }
      break;
    case DeclK:
      switch (t->kind.decl)
      {
      case VarK:
        if (st_lookup_excluding_parent(cur_scope,t->attr.name) != NULL){
          fprintf(listing, "Error: Symbol \"%s\" is redefined at line %d\n", t->attr.name, t->lineno);
        }
        st_insert(cur_scope,t->attr.name,t->type,t->lineno,cur_scope->location++,Var);
        break;
      case ParaK:
        if (t->attr.name != NULL){
          if (st_lookup_excluding_parent(cur_scope,t->attr.name) != NULL){
            fprintf(listing, "Error: Symbol \"%s\" is redefined at line %d\n", t->attr.name, t->lineno);
          }
          BucketList b = st_lookup(cur_scope, cur_scope->name);
          Para p = (Para) malloc (sizeof(struct Para_t));
          p->type = t->type;
          p->next = NULL;
          if (b->para == NULL){
            b->para = p;
          } else {
            Para k = b->para;
            while (k->next != NULL)
            {
            
              k = k->next;
            }
            k->next = p;
          }
          // printf("%s\n",cur_scope->name);
          st_insert(cur_scope,t->attr.name,t->type,t->lineno,cur_scope->location++,Var);
          // add_para(cur_scope,t->attr.name,t->type);
        }
        break;
      case FuncK:
        if (st_lookup_excluding_parent(cur_scope,t->attr.name) != NULL){
          fprintf(listing, "Error: Symbol \"%s\" is redefined at line %d\n", t->attr.name, t->lineno);
        }
        func_comp = 1;
        // if (t->type == INT)
        //   st_insert(cur_scope,t->attr.name,IntFunc,t->lineno,cur_scope->location++);
        // else
        //   st_insert(cur_scope,t->attr.name,VoidFunc,t->lineno,cur_scope->location++);
        st_insert(cur_scope,t->attr.name,t->type,t->lineno,cur_scope->location++,Func);
        ScopeList s = insert_scope(t->attr.name);
        t->scope = s;
        s->parent = cur_scope;
        if (cur_scope->child == NULL)
          cur_scope->child = s;
        else {
          ScopeList k = cur_scope->child;
          while (k->next_scope != NULL)
          {
            k = k->next_scope;
          }
          k->next_scope = s;
        }
        // add_func(t->attr.name,t->type);
        stack_push(s);
        break;
      default:
        break;
      }
    default:
      break;
  }
}

/* Function buildSymtab constructs the symbol 
 * table by preorder traversal of the syntax tree
 */
void buildSymtab(TreeNode * syntaxTree)
{ 
  ScopeList s = insert_scope("Glabal");
  global = s;
  stack_push(s);
  st_insert(s,"input",Integer,0,s->location++,Func);
  st_insert(s,"output",Void,0,s->location++,Func);
  BucketList b = st_lookup(s, "output");
  Para p = (Para) malloc (sizeof(struct Para_t));
  p->type = Integer;
  p->next = NULL;
  if (b->para == NULL){
    b->para = p;
  } else {
    Para k = b->para;
    while (k->next != NULL)
    {
    
      k = k->next;
    }
    k->next = p;
  }
  traverse(syntaxTree,insertNode,pop_scope);
  if (TraceAnalyze)
  { fprintf(listing,"\nSymbol table:\n\n");
    printSymTab(listing);
  }
}

static void typeError(TreeNode * t, char * message)
{ fprintf(listing,"Type error at line %d: %s\n",t->lineno,message);
  Error = TRUE;
}

void push_scope(TreeNode *t){
  switch (t->nodekind)
  {
  case StmtK:
    if (t->kind.stmt == CompK){
      if (func_comp)
        func_comp = 0;
      else
        stack_push(t->scope);
    } 
    break;
  case DeclK:
    if (t->kind.decl == FuncK){
      stack_push(t->scope);
      rt_push(t->type);
      func_comp = 1;
    }
    break;
  default:
    break;
  }
}

int is_int(ScopeList scope, TreeNode *t){
  if (t->type == Integer){
    return 1;
  } else 
    return 0;
}

int check_para(Para p, TreeNode *c){
  if (p == NULL && c == NULL)
    return 1;
  if (p == NULL || c == NULL)
    return 0;
  if (p->type == c->type){
    return check_para(p->next,c->sibling);
  } else {
    return 0;
  }
}

/* Procedure checkNode performs
 * type checking at a single tree node
 */
static void checkNode(TreeNode * t)
{ 
  ScopeList cur_scope = stack_top();
  // printf("%d",sp);
  // printf("%d\n",t->lineno);
  // printf("%s\n",cur_scope->name);
  BucketList b;
  switch (t->nodekind)
  { case StmtK:
      switch (t->kind.stmt)
      { 
        case IfK :
          if (!is_int(cur_scope,t->child[0])){
            fprintf(listing, "Error: invalid condition at line %d\n", t->child[0]->lineno);
          }
          break;
        case If_ElseK :
          if (!is_int(cur_scope,t->child[0])){
            fprintf(listing, "Error: invalid condition at line %d\n", t->child[0]->lineno);
          }
          break;
        case ReturnK :
          if (t->type != rt_top() || (t->type == Integer && !is_int(cur_scope,t->child[0])))
            fprintf(listing, "Error: Invalid return at line %d\n", t->lineno);
          break;
        case WhileK :
          if (!is_int(cur_scope,t->child[0])){
            fprintf(listing, "Error: invalid condition at line %d\n", t->child[0]->lineno);
          }
          break;
        case CompK :
        default:
          break;
      }
      break;
    case ExpK:
      switch (t->kind.exp)
      { 
        case BiOpK:
          t->type = Integer;
          if (!is_int(cur_scope,t->child[0]) || !is_int(cur_scope, t->child[1]))
            fprintf(listing, "Error: invalid operation at line %d\n", t->lineno);
          break;
        case ConstK:
          t->type = Integer;
          break;
        case CallK:
          b = st_lookup(cur_scope,t->attr.name);
          // printf("%s\n",cur_scope->name);
          if (b == NULL || b->kind != Func){
            fprintf(listing, "Error: undeclared function \"%s\" is called at line %d\n", t->attr.name, t->lineno);
            break;
          }
          t->type = b->type;
          Para f = b->para;
          TreeNode *child = t->child[0];
          if (!check_para(f,child))
            fprintf(listing, "Error: Invalid function call at line %d (name : \"%s\")\n", t->lineno, t->attr.name);
          break;
        case AssignK:
          t->type = Integer;
          if (!is_int(cur_scope,t->child[0]) || !is_int(cur_scope,t->child[1]))
            fprintf(listing, "Error: invalid assignment at line %d\n", t->lineno);
          break;
        case VarAccK:
          // printf("%d\n", t->lineno);
          b = st_lookup(cur_scope,t->attr.name);
          if (b == NULL || b->kind != Var){
            fprintf(listing, "Error: undeclared variable \"%s\" is used at line %d\n", t->attr.name, t->lineno);
            // break;
          } else if (b->type == Integer && t->child[0] != NULL){
            fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). indexing can only allowed for int[] variables\n", t->lineno, t->attr.name);
            // break;
          } else if (b->type == IntArr && (t->child[0] == NULL || !is_int(cur_scope,t->child[0]))){
            fprintf(listing, "Error: Invalid array indexing at line %d (name : \"%s\"). indicies should be integer\n", t->lineno, t->attr.name);
            t->type = b->type;
            // break;
          }
          t->type = Integer;
          break;
        default:
          break;
      }
      break;
    case DeclK:
      switch (t->kind.decl)
      {
      case VarK:
        if (t->type == Void || t->type == VoidArr)
          fprintf(listing, "Error: The void-type variable is declared at line %d (name : \"%s\")\n", t->lineno, t->attr.name);
        break;
      case ParaK:
        break;
      case FuncK:
        break;
      default:
        break;
      }
    default:
      break;
  }
  pop_scope(t);
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal
 */
void typeCheck(TreeNode * syntaxTree)
{ i = 0;
  sp = 0;
  stack_push(global);
  traverse(syntaxTree,push_scope,checkNode);
}
