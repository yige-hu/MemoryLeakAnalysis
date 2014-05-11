#include <stdlib.h>

/*
 * safe
 */
int main(int argc, char **argv) {
  int x = 15;
  int y = x;
  int *a, *b, *c, *d;
  a = (int*) malloc(sizeof(int));
  if (x <10) {
    b = a;
  } else {
    d = a;
  }
  *b = x;
  a = c;

  return 0;
}
