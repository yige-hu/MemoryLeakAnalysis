#include <stdlib.h>

/*
 * safe
 */
int *test() {
  int a;
  int *x, *y, *z, *t, *s;
  x = (int*) malloc(sizeof(int));
  y = (int*) malloc(sizeof(int));
  while ((a++) < 100) {
    t = x;
    x = y;
    y = z;
    z = NULL;
  }

  for (int i = 100; i < 0; i--) {
    z = x;
    x = y;
    y = t;
    t = NULL;
  }
  return y;
}
