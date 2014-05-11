#include <stdlib.h>

/* 
 * unsafe at:
 * a = c;
 *
 * safe at:
 * a = NULL;
 * a = c;
 */
int main(int argc, char **argv) {
  int *a, *c;
  a = (int*) malloc(sizeof(int));
  a = c;
  free(a);
  a = NULL;
  a = c;

  return 0;
}
