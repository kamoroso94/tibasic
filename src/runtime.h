#ifndef RUNTIME_H
#define RUNTIME_H

#include "ast.h"

extern ast_t *program;

void runtimeError(const char *str);
void eval(ast_t *program);

#endif
