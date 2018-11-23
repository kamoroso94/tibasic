%{
#include <stdio.h>
#include "runtime.h"
#include "ast.h"
#include "nodes.h"
#include "y.tab.h"

extern int yyline, yycolumn;

void yyerror(const char *str);
%}

%start Program

%union {
  int ival;
  double dval;
  ast_t *tptr;
};

%token IF_TK THEN_TK ELSE_TK FOR_TK WHILE_TK REPEAT_TK END_TK PROMPT_TK DISP_TK CLRHOME_TK
%token AND_TK OR_TK XOR_TK NOT_TK ANS_TK

%token <ival> REALVAR_TK STRVAR_TK STR_TK
%token <dval> REAL_TK

%token ASSIGN_TK "->"
%token NE_TK     "!="
%token GE_TK     ">="
%token LE_TK     "<="
%token EOL_TK    ":"

%type <tptr> StatementList Statement BlockStmt IfBlock IfHeader
%type <tptr> Expression Comparison Sum Term Factor Atom Variable BlockBody
%type <tptr> ForLoop ForHeader ForArgs ForArgsStart WhileLoop WhileHeader
%type <tptr> RepeatLoop RepeatHeader LineStmt IfInline Command Assignment
%type <tptr> PromptArgList PromptArg DispArgList DispArg

%%

Program
  : StatementList
  {
    program = $1;
  }
  ;

StatementList
  : Statement
  {
    $$ = makeTree(STATEMENT_OP, &dummy, $1);
  }
  | StatementList ":" Statement
  {
    $$ = !isDummy($3) ? makeLChild($1, makeTree(STATEMENT_OP, &dummy, $3)) : $1;
  }
  ;

Statement
  : /* empty */
  {
    $$ = &dummy;
  }
  | BlockStmt
  | LineStmt
  ;

BlockStmt
  : IfBlock
  | ForLoop
  | WhileLoop
  | RepeatLoop
  ;

IfBlock
  : IfHeader THEN_TK ":" BlockBody END_TK
  {
    $$ = makeRChild($1, makeTree(BRANCH_OP, $4, &dummy));
  }
  | IfHeader THEN_TK ":" BlockBody ELSE_TK ":" BlockBody END_TK
  {
    $$ = makeRChild($1, makeTree(BRANCH_OP, $4, $7));
  }
  ;

IfHeader
  : IF_TK Expression ":"
  {
    $$ = makeTree(IF_OP, $2, &dummy);
  }
  ;

Expression
  : Comparison
  | Expression AND_TK Comparison
  {
    $$ = makeTree(AND_OP, $1, $3);
  }
  | Expression OR_TK Comparison
  {
    $$ = makeTree(OR_OP, $1, $3);
  }
  | Expression XOR_TK Comparison
  {
    $$ = makeTree(XOR_OP, $1, $3);
  }
  ;

Comparison
  : Sum
  | Comparison '=' Sum
  {
    $$ = makeTree(EQ_OP, $1, $3);
  }
  | Comparison "!=" Sum
  {
    $$ = makeTree(NE_OP, $1, $3);
  }
  | Comparison '>' Sum
  {
    $$ = makeTree(GT_OP, $1, $3);
  }
  | Comparison ">=" Sum
  {
    $$ = makeTree(GE_OP, $1, $3);
  }
  | Comparison '<' Sum
  {
    $$ = makeTree(LT_OP, $1, $3);
  }
  | Comparison "<=" Sum
  {
    $$ = makeTree(LE_OP, $1, $3);
  }
  ;

Sum
  : Term
  | Sum '+' Term
  {
    $$ = makeTree(ADD_OP, $1, $3);
  }
  | Sum '-' Term
  {
    $$ = makeTree(MINUS_OP, $1, $3);
  }
  ;

Term
  : Factor
  | Term Factor
  {
    $$ = makeTree(MUL_OP, $1, $2);
  }
  | Term '*' Factor
  {
    $$ = makeTree(MUL_OP, $1, $3);
  }
  | Term '/' Factor
  {
    $$ = makeTree(DIV_OP, $1, $3);
  }
  ;

Factor
  : Atom
  | '~' Factor
  {
    $$ = makeTree(MUL_OP, makeDLeaf(REAL_ND, -1), $2);
  }
  | Atom '^' Factor
  {
    $$ = makeTree(POW_OP, $1, $3);
  }
  ;

Atom
  : Variable
  | REAL_TK
  {
    $$ = makeDLeaf(REAL_ND, $1);
  }
  | NOT_TK '(' Expression ')'
  {
    $$ = makeTree(NOT_OP, $3, &dummy);
  }
  | '(' Expression ')'
  {
    $$ = $2;
  }
  ;

Variable
  : REALVAR_TK
  {
    $$ = makeILeaf(REALVAR_ND, $1);
  }
  | ANS_TK
  {
    $$ = makeLeaf(ANS_ND);
  }
  ;

BlockBody
  : /* empty */
  {
    $$ = &dummy;
  }
  | StatementList ":"
  ;

ForLoop
  : ForHeader BlockBody END_TK
  {
    $$ = makeRChild($1, $2);
  }
  ;

ForHeader
  : FOR_TK '(' ForArgs ')' ":"
  {
    $$ = makeTree(FOR_OP, $3, &dummy);
  }
  ;

ForArgs
  : ForArgsStart
  | ForArgsStart ',' Expression
  {
    $$ = makeRChild($1, $3);
  }
  ;

ForArgsStart
  : REALVAR_TK ',' Expression ',' Expression
  {
    $$ = makeTree(
      COMMA_OP,
      makeTree(
        ASSIGN_OP,
        makeILeaf(REALVAR_ND, $1),
        $3
      ),
      makeTree(
        COMMA_OP,
        $5,
        &dummy
      )
    );
  }
  ;

WhileLoop
  : WhileHeader BlockBody END_TK
  {
    $$ = makeRChild($1, $2);
  }
  ;

WhileHeader
  : WHILE_TK Expression ":"
  {
    $$ = makeTree(WHILE_OP, $2, &dummy);
  }
  ;

RepeatLoop
  : RepeatHeader BlockBody END_TK
  {
    $$ = makeRChild($1, $2);
  }
  ;

RepeatHeader
  : REPEAT_TK Expression ":"
  {
    $$ = makeTree(REPEAT_OP, $2, &dummy);
  }
  ;

LineStmt
  : IfInline
  | Command
  | Expression
  {
    $$ = makeTree(ASSIGN_OP, makeLeaf(ANS_ND), $1);
  }
  | STR_TK { $$ = &dummy; }
  | STRVAR_TK { $$ = &dummy; }
  ;

IfInline
  : IfHeader LineStmt
  {
    $$ = makeRChild(
      $1,
      makeTree(
        BRANCH_OP,
        makeTree(
          STATEMENT_OP,
          &dummy,
          $2
        ),
        &dummy
      )
    );
  }
  ;

Command
  : Assignment
  {
    $$ = makeTree(ASSIGN_OP, makeLeaf(ANS_ND), $1);
  }
  | PROMPT_TK PromptArgList
  {
    $$ = makeTree(PROMPT_OP, &dummy, $2);
  }
  | DISP_TK DispArgList
  {
    $$ = makeTree(DISP_OP, &dummy, $2);
  }
  | CLRHOME_TK
  {
    $$ = makeLeaf(CLRHOME_OP);
  }
  ;

Assignment
  : Expression "->" Variable
  {
    $$ = makeTree(ASSIGN_OP, $3, $1);
  }
  ;

PromptArgs
  : PromptArg
  {
    $$ = makeTree(COMMA_OP, &dummy, $1);
  }
  | PromptArgs ',' PromptArg
  {
    $$ = makeLChild($1, makeTree(COMMA_OP, &dummy, $3));
  }
  | 
  ;

PromptArg
  : REALVAR_TK
  {
    makeILeaf(REALVAR_ND, $1);
  }
  | STRVAR_TK
  {
    makeILeaf(STRVAR_ND, $1);
  }
  ;

DispArgs
  : DispArg
  {
    $$ = makeTree(COMMA_OP, &dummy, $1);
  }
  | DispArgs ',' DispArg
  {
    $$ = makeLChild($1, makeTree(COMMA_OP, &dummy, $3));
  }
  ;

DispArg
  : Expression
  | STR_TK
  {
    $$ = makeILeaf(STR_ND, $1);
  }
  | STRVAR_TK
  {
    $$ = makeILeaf(STRVAR_ND, $1);
  }
  ;

%%

void yyerror(const char *str) {
  fprintf(stderr, "Syntax error: %s\n\tat (%d,%d)\n", str, yyline, yycolumn);
}
