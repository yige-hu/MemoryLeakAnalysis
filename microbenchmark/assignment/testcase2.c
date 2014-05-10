#include <stdlib.h>

/* unsafe at:
 * a = c;
 * a = NULL;
 */
int main(int argc, char **argv) {
  int *a, *c;
  a = (int*) malloc(sizeof(int));
  a = c;
  a = NULL;

  return 0;
}
