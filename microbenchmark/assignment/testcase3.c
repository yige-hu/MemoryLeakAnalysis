#include <stdlib.h>

/*
 * unsafe: b = c
 */
int main(int argc, char **argv) {
  int *a, *b, *c;
  a = (int*) malloc(sizeof(int));
  b = a;
  a = c;
  b = c;

  return 0;
}
