#include <stdlib.h>

/*
 * safe
 */
int main(int argc, char **argv) {
  int *a, *b, *c, *d, *e;
  a = (int*) malloc(sizeof(int));
  b = (int*) malloc(sizeof(int));

  if (e == a) {
    *a = 12;
  } else {
    e = a;
  }
  a = c;

  if (d != b) {
    d = b;
  }
  b = c;

  free(d);
  free(e);

  return 0;
}
