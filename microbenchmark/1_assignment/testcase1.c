#include <stdlib.h>

/*
 * safe
 */
int main(int argc, char **argv) {
  int x = 15;
  int y = x;
  int *a, *b, *c;
  a = (int*) malloc(sizeof(int));
  free(c);
  b = a;
  *b = x;
  a = c;
  free(a);
  return 0;
}
