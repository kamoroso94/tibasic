#ifndef TI_TYPES_H
#define TI_TYPES_H

#define LIST_BUF_SIZE 999

typedef struct {
  char *buffer;
  int length;
} string_t;

typedef struct {
  double buffer[LIST_BUF_SIZE];
  int length;
} list_t;

#endif
