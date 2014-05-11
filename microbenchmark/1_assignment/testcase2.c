#include <stdlib.h>

/* unsafe at:
 * a = c;
 * a = NULL;
 * b = &c;
 * d = 0;
 */
int main(int argc, char **argv) {
  int x = 0, y = 123;
  int *a, *b, *c, *d;
  a = (int*) malloc(sizeof(int));
  a = c;
  a = NULL;
  b = (int*) malloc(sizeof(int));
  *b = 456;
  x = 321 + *b;
  b = &y;
  d = c;
  d = 0;
  return 0;
}
