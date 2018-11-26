#ifndef LEXER_H
#define LEXER_H

#include "ti_types.h"

#define TBL_SIZE 256
#define HTAB_SIZE (TBL_SIZE + (TBL_SIZE >> 2))

extern string_t str_tbl[];

void free_str_tbl();

#endif
