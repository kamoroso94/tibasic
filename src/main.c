#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include "ast.h"
#include "runtime.h"
#include "lexer.h"

extern FILE *yyin;

extern int yyparse();

int main(int argc, char *argv[]) {
  int result;
  FILE *json;
  const char *fname = "ast.json";

  if(argc != 2) {
    fprintf(stderr, "Expected 1 argument\n");
    exit(EXIT_FAILURE);
  }

  // get source_file argument
  if(!(yyin = fopen(argv[1], "r"))) {
    fprintf(stderr, "Could not open file \"%s\" for input: ", argv[1]);
    perror(NULL);
    exit(EXIT_FAILURE);
  }

  // parse source_file
  hcreate(HTAB_SIZE);
  result = yyparse();
  hdestroy();
  fclose(yyin);

  // exit, no ast
  if(result) {
    free_str_tbl();
    freeTree(program);
    exit(EXIT_FAILURE);
  }

  // open ast json file
  if(!(json = fopen(fname, "w+"))) {
    fprintf(stderr, "Could not open file \"%s\" for output: ", fname);
    perror(NULL);
    free_str_tbl();
    freeTree(program);
    exit(EXIT_FAILURE);
  }

  // print ast
  printJSON(json, program);
  fputc('\n', json);
  fclose(json);
  // eval program
  eval(program);
  free_str_tbl();
  freeTree(program);
  freeRuntime();

  return 0;
}
