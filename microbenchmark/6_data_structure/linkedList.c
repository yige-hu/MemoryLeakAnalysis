#include <stdlib.h>

/*
 * unsafe at:
 * x = (List*) malloc(sizeof(List));
 */
typedef struct list {
  int data;
  struct list *next;
} List;

List *reverse(List *x) {
  List *y, *t;
  x = (List*) malloc(sizeof(List));
  y = (List*) malloc(sizeof(List));
  while (x != NULL) {
    t = x->next;
    x->next = y;
    y = x;
    x = t;
  }
  return y;
}
