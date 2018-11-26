%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <search.h>
#include "ast.h"
#include "y.tab.h"
#include "lexer.h"
#include "ti_types.h"

#define CONSUME (yycolumn += yyleng)
#define WRAP_LINE (yyline++, yycolumn = 1)

extern YYSTYPE yylval;
extern int yyleng;
int yyline = 1;
int yycolumn = 1;
static char errmsg[25];
static string_t temp_str;
string_t str_tbl[TBL_SIZE];
static int tbl_len = 0;

extern void yyerror(const char *str);
int yywrap();
static int add_string(string_t *str);
%}

%x COMMENT STRING

newline \r|\n|\r\n
digit   [0-9]
num     {digit}+
real    {num}?"."{num}|{num}"."?
badReal {digit}*("."{digit}*){2,}

%%

"If "       { CONSUME; return IF_TK; }
"Then"      { CONSUME; return THEN_TK; }
"Else"      { CONSUME; return ELSE_TK; }
"For"       { CONSUME; return FOR_TK; }
"While "    { CONSUME; return WHILE_TK; }
"Repeat "   { CONSUME; return REPEAT_TK; }
"End"       { CONSUME; return END_TK; }
"Prompt "   { CONSUME; return PROMPT_TK; }
"Disp "     { CONSUME; return DISP_TK; }
"ClrHome"   { CONSUME; return CLRHOME_TK; }
"Ans"       { CONSUME; return ANS_TK; }
"Str"[0-9]  {
              CONSUME;
              yylval.ival = yytext[3] - '0';
              return STRVAR_TK;
            }

" and "     { CONSUME; return AND_TK; }
" or "      { CONSUME; return OR_TK; }
" xor "     { CONSUME; return XOR_TK; }
"not"       { CONSUME; return NOT_TK; }

"->"            { CONSUME; return ASSIGN_TK; }
">="            { CONSUME; return GE_TK; }
"<="            { CONSUME; return LE_TK; }
"!="            { CONSUME; return NE_TK; }
[-+*\/~^=><(),] { CONSUME; return yytext[0]; }

":"       { CONSUME;   return EOL_TK; }
{newline} { WRAP_LINE; return EOL_TK; }

"#"       { CONSUME; BEGIN COMMENT; }
"\""      {
            CONSUME;
            temp_str.buffer = calloc(1, sizeof(char));
            temp_str.length = 0;
            BEGIN STRING;
          }

[A-Z]     {
            CONSUME;
            yylval.ival = yytext[0] - 'A';
            return REALVAR_TK;
          }

{badReal} { CONSUME; yyerror("Number formatted incorrectly"); }
{real}    {
            CONSUME;
            yylval.dval = atof(yytext);
            return REAL_TK;
          }

.         {
            CONSUME;
            sprintf(errmsg, "Unexpected character '%c'", yytext[0]);
            yyerror(errmsg);
          }

<COMMENT>[^\r\n]+   { CONSUME; }
<COMMENT>{newline}  { WRAP_LINE; BEGIN INITIAL; }

<STRING>[^\r\n\"]+  {
                      CONSUME;
                      free(temp_str.buffer);
                      temp_str.buffer = strdup(yytext);
                      temp_str.length = yyleng;
                    }
<STRING>{newline}   {
                      WRAP_LINE;
                      free(temp_str.buffer);
                      yyerror("Unclosed string literal");
                      BEGIN INITIAL;
                    }
<STRING>"\""        {
                      CONSUME;
                      yylval.ival = add_string(&temp_str);
                      BEGIN INITIAL;
                      return STR_TK;
                    }

%%

// check for unclosed string literals
int yywrap() {
  if(YYSTATE == STRING) {
    yyerror("Unclosed string literal");
    free(temp_str.buffer);
  }
  return 1;
}

static int add_string(string_t *str) {
  ENTRY *found;
  ENTRY item = {
    .key = str->buffer,
    .data = NULL
  };

  if((found = hsearch(item, FIND)) != NULL) {
    free(str->buffer);
    return (string_t*)found->data - str_tbl;
  }

  if(tbl_len >= TBL_SIZE) {
    yyerror("String table overflow");
    return -1;
  }

  str_tbl[tbl_len].buffer = str->buffer;
  str_tbl[tbl_len].length = str->length;
  item.data = (void*)(str_tbl + tbl_len);
  hsearch(item, ENTER);
  return tbl_len++;
}

void free_str_tbl() {
  int i;
  for(i = 0; i < tbl_len; i++) {
    free(str_tbl[i].buffer);
    str_tbl[i].buffer = NULL;
  }
  tbl_len = 0;
}
