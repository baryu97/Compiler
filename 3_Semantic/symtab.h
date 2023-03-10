/****************************************************/
/* File: symtab.h                                   */
/* Symbol table interface for the TINY compiler     */
/* (allows only one symbol table)                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include "globals.h"

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */

/* SIZE is the size of the hash table */
#define SIZE 211

/* the list of line numbers of the source 
 * code in which a variable is referenced
 */
typedef struct LineListRec
   { int lineno;
     struct LineListRec * next;
   } * LineList;

/* The record in the bucket lists for
 * each variable, including name, 
 * assigned memory location, and
 * the list of line numbers in which
 * it appears in the source code
 */

typedef enum {Func,Var} SymbolKind;

typedef struct Para_t
{
  ExpKind type;
  struct Para_t *next;
} * Para;


typedef struct BucketListRec
   { char * name;
     ExpType type;
     SymbolKind kind;
     LineList lines;
     int memloc ; /* memory location for variable */
     struct BucketListRec * next;
     Para para;
   } * BucketList;

typedef struct ScopeListRec
{
  char * name;
  int location;
  BucketList bucket[SIZE];
  struct ScopeListRec * parent;
  struct ScopeListRec * child;
  struct ScopeListRec * next;
  struct ScopeListRec * next_scope;
} * ScopeList;

ScopeList insert_scope(char *name);

void st_insert( ScopeList scope, char * name, ExpType type, int lineno, int loc, SymbolKind kind);

/* Function st_lookup returns the memory 
 * location of a variable or -1 if not found
 */
BucketList st_lookup (ScopeList scope, char *name);
BucketList st_lookup_excluding_parent (ScopeList scope, char *name);

/* Procedure printSymTab prints a formatted 
 * listing of the symbol table contents 
 * to the listing file
 */
void printSymTab(FILE * listing);

#endif
