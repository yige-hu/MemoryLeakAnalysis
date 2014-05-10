#include <stdlib.h>

/*
 * safe
 */
int main(int argc, char **argv) {
  int *a, *b, *c;
  b = (int*) malloc(sizeof(int));
  a = c;

  free(b);

  return 0;
}
