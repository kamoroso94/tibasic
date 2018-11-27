#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include "runtime.h"
#include "ast.h"
#include "nodes.h"
#include "lexer.h"
#include "ti_types.h"

#define INTERNAL_ERROR(S) (fatalError("Internal error: " S))
#define RUNTIME_ERROR(S) (fatalError("Runtime error: " S))

static double realvars[26];
static string_t strvars[10];
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
static void readString(string_t *str);

static int cmp(double a, double b) { return (a > b) - (a < b); }
static int sign(double a) { return cmp(a, 0); }

void freeRuntime() {
  int i;
  for(i = 0; i < 10; i++) {
    free(strvars[i].buffer);
  }
}

// exit with error
static void fatalError(const char *str) {
  // TODO: supply (line,column)
  free_str_tbl();
  freeTree(program);
  freeRuntime();
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
  if(isDummy(ifStmt->left)) INTERNAL_ERROR("If condition missing!");
  if(isDummy(ifStmt->right)) INTERNAL_ERROR("If branches missing!");

  if(evalExpr(ifStmt->left)) {
    eval(ifStmt->right->left);
  } else {
    eval(ifStmt->right->right);
  }
}

// eval expression
static double evalExpr(ast_t *expr) {
  if(isDummy(expr)) INTERNAL_ERROR("Expression missing!");

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
  double lval, rval, result;
  switch(expr->opType) {
    case POW_OP:
    lval = evalExpr(expr->left);
    rval = evalExpr(expr->right);
    if(lval == 0 && rval == 0) RUNTIME_ERROR("Zero to the power of zero");
    errno = 0;
    result = pow(lval, rval);
    if(errno) RUNTIME_ERROR("Domain error");
    return result;

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

  INTERNAL_ERROR("Bad Atom type!");
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
  ast_t *assign, *comma;
  double *realvar, startVal, endVal, delta;
  int dsign;

  if(isDummy(forStmt->left)) INTERNAL_ERROR("For header missing!");
  if(isDummy(forStmt->left->left)) INTERNAL_ERROR("For initializer missing!");
  if(isDummy(forStmt->left->right)) INTERNAL_ERROR("For range missing!");

  assign = forStmt->left->left;
  comma = forStmt->left->right;
  realvar = realvars + assign->left->value.ival;
  startVal = evalExpr(assign->right);
  endVal = evalExpr(comma->left);
  delta = !isDummy(comma->right) ? evalExpr(comma->right) : 1;
  dsign = sign(delta);

  if(delta == 0) RUNTIME_ERROR("Increment must be nonzero");

  *realvar = startVal;
  while(cmp(*realvar, endVal) != dsign) {
    eval(forStmt->right);
    *realvar += delta;
  }
}

// eval while loop
static void evalWhile(ast_t *whileStmt) {
  if(isDummy(whileStmt->left)) INTERNAL_ERROR("While condition missing!");

  while(evalExpr(whileStmt->left)) {
    eval(whileStmt->right);
  }
}

// eval repeat loop
static void evalRepeat(ast_t *repeatStmt) {
  if(isDummy(repeatStmt->left)) INTERNAL_ERROR("Repeat condition missing!");

  do {
    eval(repeatStmt->right);
  } while(!evalExpr(repeatStmt->left));
}

// eval command
static void evalCmd(ast_t *cmd) {
  switch(cmd->opType) {
    case ASSIGN_OP:
    evalAssign(cmd);
    break;

    case PROMPT_OP:
    evalPrompt(cmd);
    break;

    case DISP_OP:
    evalDisp(cmd);
    break;

    case CLRHOME_OP:
    system("clear");
    break;

    default:
    printJSON(stdout, cmd);
    INTERNAL_ERROR("Bad command type!");
  }
}

// eval user prompt
static void evalPrompt(ast_t *prompt) {
  ast_t *node = prompt->right;
  ast_t *arg;

  if(isDummy(node)) INTERNAL_ERROR("Prompt missing args!");

  while(!isDummy(node)) {
    arg = node->right;
    if(isDummy(arg)) INTERNAL_ERROR("Prompt arg undefined!");

    switch(arg->nodeKind) {
      case REALVAR_ND:
      printf("%c=?", 'A' + arg->value.ival);
      realvars[arg->value.ival] = readDouble();
      break;

      case STRVAR_ND:
      printf("Str%c=?", '0' + arg->value.ival);
      readString(strvars + arg->value.ival);
      break;

      default:
      INTERNAL_ERROR("Bad Prompt arg type!");
    }

    node = node->left;
  }
}

// eval display command
static void evalDisp(ast_t *disp) {
  if(isDummy(disp->right)) INTERNAL_ERROR("Disp missing args!");
  ast_t *node = disp->right;
  ast_t *arg;

  while(!isDummy(node)) {
    arg = node->right;
    if(isDummy(arg)) INTERNAL_ERROR("Disp arg undefined!");

    switch(arg->nodeKind) {
      case EXPR_ND:
      case REAL_ND:
      case REALVAR_ND:
      printf("%16.10G\n", evalExpr(arg));
      break;

      case STR_ND:
      printf("%s\n", str_tbl[arg->value.ival].buffer);
      break;

      case STRVAR_ND:
      printf("%s\n", strvars[arg->value.ival].buffer);
      break;

      default:
      INTERNAL_ERROR("Bad Disp arg type!");
    }

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
  if(result != EOF && result != '\n') {
    RUNTIME_ERROR("Number formatted incorrectly");
  }

  return d;
}

static void readString(string_t *str) {
  char *line = NULL;
  size_t len = 0;
  int result;

  while((result = getline(&line, &len, stdin)) <= 0 || len == 0 || line[0] == '\n') {
    free(line);
    if(result == -1) RUNTIME_ERROR("Error reading input");
    line = NULL;
    len = 0;
  }

  len = strnlen(line, len);
  if(line[len - 1] == '\n') {
    line[--len] = '\0';
  }
  str->buffer = line;
  str->length = (int)len;
}
