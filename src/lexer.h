#ifndef LEXER_H
#define LEXER_H

#define TBL_SIZE 256
#define HTAB_SIZE (TBL_SIZE + (TBL_SIZE >> 2))

extern char *str_tbl[];

void free_str_tbl();

#endif
