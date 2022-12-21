/****************************************************/
/* File: symtab.c                                   */
/* Symbol table implementation for the TINY compiler*/
/* (allows only one symbol table)                   */
/* Symbol table is implemented as a chained         */
/* hash table                                       */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

ScopeList global_scope = NULL;

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

/* the hash table */
static ScopeList hashTable[SIZE];

ScopeList insert_scope(char *name){
  ScopeList s = (ScopeList) malloc (sizeof (struct ScopeListRec));
  s->name = name;
  for (size_t i = 0; i < SIZE; i++)
  {
    s->bucket[i] = NULL;
  }
  s->location = 0;
  s->parent = NULL;
  s->next_scope = NULL;
  s->child = NULL;
  s->next = NULL;
  if (global_scope == NULL){
    global_scope = s;  
  }
  return s;
}

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert( ScopeList scope, char * name, ExpType type, int lineno, int loc, SymbolKind kind )
{ 
  int h = hash(name);
  BucketList l = scope->bucket[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) /* variable not yet in table */
  { l = (BucketList) malloc(sizeof(struct BucketListRec));
    l->name = name;
    l->lines = (LineList) malloc(sizeof(struct LineListRec));
    l->lines->lineno = lineno;
    l->memloc = loc;
    l->kind = kind;
    l->type = type;
    l->lines->next = NULL;
    l->next = scope->bucket[h];
    scope->bucket[h] = l; }
  else /* found in table, so just add line number */
  { LineList t = l->lines;
    while (t->next != NULL) t = t->next;
    t->next = (LineList) malloc(sizeof(struct LineListRec));
    t->next->lineno = lineno;
    t->next->next = NULL;
  }
} /* st_insert */

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
BucketList st_lookup (ScopeList scope, char *name)
{ 
  // int sh = hash(scope);
  // int h = hash(name);
  // ScopeList sl =  hashTable[sh];
  // while ((sl != NULL) && (strcmp(scope,sl->name) != 0))
  //   sl = sl->next;
  // // if (sh == NULL)
  // BucketList l = sl->bucket[h];
  // while ((l != NULL) && (strcmp(name,l->name) != 0))
  //   l = l->next;
  // if (l == NULL) return NULL;
  // else return l;
  while (scope->parent != NULL)
  {
    BucketList b;
    if ((b = st_lookup_excluding_parent(scope,name)) == NULL){
      scope = scope->parent;
    } else {
      return b;
    }
  }
  return st_lookup_excluding_parent(scope,name);
}
 
BucketList st_lookup_excluding_parent (ScopeList scope, char *name){
  int h = hash(name);
  BucketList l = scope->bucket[h];
  while ((l != NULL) && (strcmp(name,l->name) != 0))
    l = l->next;
  if (l == NULL) return NULL;
  else return l;
}

void print_symbols(ScopeList s){
  for (int i=0;i<SIZE;++i)
  { if (s->bucket[i] != NULL)
    { BucketList l = s->bucket[i];
      while (l != NULL)
      { LineList t = l->lines;
        fprintf(listing,"%-14s ",l->name);
        fprintf(listing,"%-12d  ",l->kind);
        fprintf(listing,"%-12d  ",l->type);
        fprintf(listing,"%-12s  ",s->name);
        fprintf(listing,"%-12d  ",l->memloc);
        while (t != NULL)
        { fprintf(listing,"%4d ",t->lineno);
          t = t->next;
        }
        fprintf(listing,"\n");
        l = l->next;
      }
    }
  }
}

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing)
{ int i;
  fprintf(listing, "\n< Symbol Table >\n");
  fprintf(listing,"Symbol Name   Symbol Kind   Symbol Type    Scope Name   Location  Line Numbers\n");
  fprintf(listing,"-------------  -----------  -------------  ------------  --------  ------------\n");
  ScopeList s = global_scope;
  print_symbols(s);
  s = s->child;
  while (s != NULL)
  {
    print_symbols(s);
    s = s->next_scope;
  }
  
  
} /* printSymTab */
