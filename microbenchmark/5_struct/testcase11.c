#include <stdlib.h>

/*
 * unsafe:
 * x = (List*) malloc(sizeof(List));
 * y = NULL;
 */
typedef struct list {
  int data;
  struct list *next;
  int extra;
} List;

List *reverse(List *x) {
  List *y, *t;
  x = (List*) malloc(sizeof(List));
  y = (List*) malloc(sizeof(List));
  y = NULL;
  x->data = 15;
  x->extra = 12;
  while (x != NULL) {
    t = x->next;
    x->next = y;
    y = x;
    x = t;
  }

  free(x);
  return y;
}
