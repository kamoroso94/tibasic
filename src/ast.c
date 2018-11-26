#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "nodes.h"

// dummy node
ast_t dummy = {
  .left = NULL,
  .right = NULL,
  .nodeKind = DUMMY_ND,
  .opType = 0,
  .value = { .ival = 0 }
};

// check if node is dummy
int isDummy(ast_t *node) {
  return node == NULL || node->nodeKind == DUMMY_ND;
}

// check if node is leaf
int isLeaf(ast_t *node) {
  return isDummy(node) || isDummy(node->left) && isDummy(node->right);
}

// make leaf node
ast_t *makeLeaf(int nodeKind) {
  ast_t *node;

  if(nodeKind == DUMMY_ND) return &dummy;

  node = calloc(1, sizeof(ast_t));
  node->nodeKind = nodeKind;
  node->left = &dummy;
  node->right = &dummy;

  return node;
}

// make int leaf node
ast_t *makeILeaf(int nodeKind, int ival) {
  ast_t *node = makeLeaf(nodeKind);
  node->value.ival = ival;
  return node;
}

// make double leaf node
ast_t *makeDLeaf(int nodeKind, double dval) {
  ast_t *node = makeLeaf(nodeKind);
  node->value.dval = dval;
  return node;
}

// make tree node
ast_t *makeTree(int opType, ast_t *left, ast_t *right) {
  ast_t *node = makeLeaf(EXPR_ND);

  node->opType = opType;
  node->left = left;
  node->right = right;

  return node;
}

// set leftmost descendant
ast_t *makeLChild(ast_t *root, ast_t *child) {
  ast_t *node;

  if(isDummy(root)) return child;
  if(isDummy(child)) return root;

  node = root;
  while(!isDummy(node->left)) {
    node = node->left;
  }

  node->left = child;
  return root;
}

// set rightmost descendant
ast_t *makeRChild(ast_t *root, ast_t *child) {
  ast_t *node;

  if(isDummy(root)) return child;
  if(isDummy(child)) return root;

  node = root;
  while(!isDummy(node->right)) {
    node = node->right;
  }

  node->right = child;
  return root;
}

// free tree memory
void freeTree(ast_t *root) {
  if(isDummy(root)) return;

  if(!isLeaf(root)) {
    freeTree(root->left);
    root->left = NULL;
    freeTree(root->right);
    root->right = NULL;
  }

  free(root);
}

// print tree in JSON
void printJSON(FILE *out, ast_t *root) {
  if(root == NULL) {
    fprintf(out, "null");
    return;
  }

  fprintf(out, "{\"nodeKind\":%d,\"opType\":%d,\"value\":{", root->nodeKind, root->opType);
  if(root->nodeKind == REAL_ND) {
    fprintf(out, "\"dval\":%g", root->value.dval);
  }
  if(root->nodeKind == REALVAR_ND) {
    fprintf(out, "\"ival\":%d", root->value.ival);
  }

  fprintf(out, "},\"left\":");

  printJSON(out, root->left);
  fprintf(out, ",\"right\":");
  printJSON(out, root->right);
  fprintf(out, "}");
}
