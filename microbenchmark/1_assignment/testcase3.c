#include <stdlib.h>

/*
 * unsafe: d = NULL
 */
int main(int argc, char **argv) {
  int *a, *b, *c, *d;
  a = (int*) malloc(sizeof(int));
  b = a;
  c = b;
  d = c;
  a = NULL;
  b = NULL;
  c = NULL;
  d = NULL;

  return 0;
}
