#include <stdlib.h>

/*
 * unsafe at: a = c;
 */
int main(int argc, char **argv) {
  int x = 15;
  int y = x;
  int *a, *b, *c;
  a = (int*) malloc(sizeof(int));
  if (x <10) {
    b = a;
  }
  *b = x;
  a = c;

  return 0;
}
