#ifndef AST_H
#define AST_H

#include <stdio.h>

typedef struct node {
  struct node *left, *right;
  int nodeKind, opType;
  union {
    int ival;
    double dval;
  } value;
} ast_t;

extern ast_t dummy;

int isDummy(ast_t *node);
int isLeaf(ast_t *node);
ast_t *makeLeaf(int nodeKind);
ast_t *makeILeaf(int nodeKind, int ival);
ast_t *makeDLeaf(int nodeKind, double dval);
ast_t *makeTree(int opType, ast_t *left, ast_t *right);
ast_t *makeLChild(ast_t *root, ast_t *child);
ast_t *makeRChild(ast_t *root, ast_t *child);
void freeTree(ast_t *root);
void printJSON(FILE *out, ast_t *root);
// void printCode(ast_t *root);

#endif
