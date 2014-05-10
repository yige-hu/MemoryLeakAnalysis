#include <stdlib.h>

typedef struct list {
  int data;
  struct list *next;
} List;

List *reverse(List *x) {
  List *y, *t;
  x = (List*) malloc(sizeof(List));
  y = (List*) malloc(sizeof(List));
  y = NULL;
  while (x != NULL) {
    //int a = x->data;
    t = x->next;
    x->next = y;
    y = x;
    x = t;
  }
  return y;
}
