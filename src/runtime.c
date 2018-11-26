#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "runtime.h"
#include "ast.h"
#include "nodes.h"

#define INTERNAL_ERROR(S) (fatalError("Internal error: " S))
#define RUNTIME_ERROR(S) (fatalError("Runtime error: " S))

static double realvars[26];
static double ansvar;
ast_t *program = &dummy;

static void fatalError(const char *str);
static void evalStmt(ast_t *stmt);
static void evalIf(ast_t *ifStmt);
static double evalExpr(ast_t *expr);
static double evalComp(ast_t *expr);
static double evalSum(ast_t *expr);
static double evalTerm(ast_t *expr);
static double evalFactor(ast_t *expr);
static double evalAtom(ast_t *expr);
static double evalAssign(ast_t *assign);
static void evalFor(ast_t *forStmt);
static void evalWhile(ast_t *whileStmt);
static void evalRepeat(ast_t *repeatStmt);
static void evalCmd(ast_t *cmd);
static void evalPrompt(ast_t *prompt);
static void evalDisp(ast_t *disp);
static double readDouble();
static char *readString();

static int cmp(double a, double b) { return (a > b) - (a < b); }
static int sign(double a) { return cmp(a, 0); }

// exit with error
static void fatalError(const char *str) {
  // TODO: supply (line,column)
  freeTree(program);
  fprintf(stderr, "%s\n", str);
  exit(EXIT_FAILURE);
}

// eval statement list
void eval(ast_t *program) {
  ast_t *node = program;
  while(!isDummy(node)) {
    evalStmt(node->right);
    node = node->left;
  }
}

// eval statement
static void evalStmt(ast_t *stmt) {
  if(isDummy(stmt)) return;

  switch(stmt->opType) {
    case IF_OP:
    return evalIf(stmt);

    case FOR_OP:
    return evalFor(stmt);

    case WHILE_OP:
    return evalWhile(stmt);

    case REPEAT_OP:
    return evalRepeat(stmt);

    default:
    return evalCmd(stmt);
  }
}

// eval if statement
static void evalIf(ast_t *ifStmt) {
  if(evalExpr(ifStmt->left)) {
    eval(ifStmt->right->left);
  } else {
    eval(ifStmt->right->right);
  }
}

// eval expression
static double evalExpr(ast_t *expr) {
  switch(expr->opType) {
    case AND_OP:
    return !!evalExpr(expr->left) & !!evalExpr(expr->right);

    case OR_OP:
    return !!evalExpr(expr->left) | !!evalExpr(expr->right);

    case XOR_OP:
    return !!evalExpr(expr->left) ^ !!evalExpr(expr->right);

    default:
    return evalComp(expr);
  }
}

// eval comparison operation
static double evalComp(ast_t *expr) {
  switch(expr->opType) {
    case EQ_OP:
    return evalExpr(expr->left) == evalExpr(expr->right);

    case NE_OP:
    return evalExpr(expr->left) != evalExpr(expr->right);

    case GT_OP:
    return evalExpr(expr->left) > evalExpr(expr->right);

    case GE_OP:
    return evalExpr(expr->left) >= evalExpr(expr->right);

    case LT_OP:
    return evalExpr(expr->left) < evalExpr(expr->right);

    case LE_OP:
    return evalExpr(expr->left) <= evalExpr(expr->right);

    default:
    return evalSum(expr);
  }
}

// eval summation
static double evalSum(ast_t *expr) {
  switch(expr->opType) {
    case ADD_OP:
    return evalExpr(expr->left) + evalExpr(expr->right);

    case MINUS_OP:
    return evalExpr(expr->left) - evalExpr(expr->right);

    default:
    return evalTerm(expr);
  }
}

// eval term of sum
static double evalTerm(ast_t *expr) {
  double rval;
  switch(expr->opType) {
    case MUL_OP:
    return evalExpr(expr->left) * evalExpr(expr->right);

    case DIV_OP:
    rval = evalExpr(expr->right);
    if(rval == 0) RUNTIME_ERROR("Division by zero");
    return evalExpr(expr->left) / rval;

    default:
    return evalFactor(expr);
  }
}

// eval factor of term
static double evalFactor(ast_t *expr) {
  double lval, rval;
  switch(expr->opType) {
    case POW_OP:
    lval = evalExpr(expr->left);
    rval = evalExpr(expr->right);
    if(lval == 0 && rval == 0) RUNTIME_ERROR("Zero to the power of zero");
    return pow(lval, rval);

    default:
    return evalAtom(expr);
  }
}

// eval atomic value
static double evalAtom(ast_t *expr) {
  if(expr->nodeKind == ANS_ND) return ansvar;
  if(expr->nodeKind == REAL_ND) return expr->value.dval;
  if(expr->nodeKind == REALVAR_ND) return realvars[expr->value.ival];

  if(expr->opType == NOT_OP) return !evalExpr(expr->left);
  if(expr->opType == ASSIGN_OP) return evalAssign(expr);

  INTERNAL_ERROR("Should not reach!");
}

// eval assignment
static double evalAssign(ast_t *assign) {
  double *lhs, rhs = evalExpr(assign->right);

  if(assign->left->nodeKind == REALVAR_ND) {
    lhs = realvars + assign->left->value.ival;
  } else {
    lhs = &ansvar;
  }

  return *lhs = rhs;
}

// eval for loop
static void evalFor(ast_t *forStmt) {
  ast_t *assign = forStmt->left->left;
  ast_t *comma = forStmt->left->right;
  double *realvar = realvars + assign->left->value.ival;
  double startVal = evalExpr(assign->right);
  double endVal = evalExpr(comma->left);
  double delta = !isDummy(comma->right) ? evalExpr(comma->right) : 1;
  int dsign = sign(delta);

  if(delta == 0) RUNTIME_ERROR("Increment must be nonzero");

  *realvar = startVal;
  while(cmp(*realvar, endVal) != dsign) {
    eval(forStmt->right);
    *realvar += delta;
  }
}

// eval while loop
static void evalWhile(ast_t *whileStmt) {
  while(evalExpr(whileStmt->left)) {
    eval(whileStmt->right);
  }
}

// eval repeat loop
static void evalRepeat(ast_t *repeatStmt) {
  do {
    eval(repeatStmt->right);
  } while(!evalExpr(repeatStmt->left));
}

// eval command
static void evalCmd(ast_t *cmd) {
  if(cmd->opType == ASSIGN_OP) evalAssign(cmd);
  if(cmd->opType == PROMPT_OP) evalPrompt(cmd);
  if(cmd->opType == DISP_OP) evalDisp(cmd);
  if(cmd->opType == CLRHOME_OP) system("clear");
}

// eval user prompt
static void evalPrompt(ast_t *prompt) {
  ast_t *node = prompt->right;

  while(!isDummy(node)) {
    printf("%c=?", 'A' + node->value.ival);
    realvars[node->value.ival] = readDouble();
    node = node->left;
  }
}

// eval display command
static void evalDisp(ast_t *disp) {
  ast_t *node = disp->right;

  while(!isDummy(node)) {
    printf("%16.10G\n", evalExpr(node->right));
    node = node->left;
  }
}

// read double
static double readDouble() {
  int result;
  double d;

  while((result = scanf("%lf", &d)) != 1) {
    if(result == EOF) RUNTIME_ERROR("Error reading input");
    if(getchar() != '\n') RUNTIME_ERROR("Number formatted incorrectly");
  }
  result = getchar();
  if(result != EOF && result != '\n') RUNTIME_ERROR("Number formatted incorrectly");

  return d;
}

static char *readString() {
  char *str = NULL;
  int len = 0;
  // TODO: implement
}
