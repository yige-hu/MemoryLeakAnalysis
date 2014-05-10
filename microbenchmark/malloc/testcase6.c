#include <stdlib.h>

/*
 * unsafe: b = (int*) malloc(sizeof(int));
 */
int main(int argc, char **argv) {
  int *a, *b, *c;
  a = (int*) malloc(sizeof(int));
  b = a;
  a = c;
  b = (int*) malloc(sizeof(int));

  return 0;
}
