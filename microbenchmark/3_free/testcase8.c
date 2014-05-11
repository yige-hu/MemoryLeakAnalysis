#include <stdlib.h>

/*
 * safe
 */
int main(int argc, char **argv) {
  int *a, *b, *c;
  b = (int*) malloc(sizeof(int));
  c = (int*) malloc(sizeof(int));
  a = c;

  free(a);
  free(b);
  free(c);

  return 0;
}
