#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  int a = 12;
  int* b;
  b = (int *) malloc(sizeof(int));
  *b = 25;
  int c = a * (*b);
  b = &a;
  free(b);
  return c;
}
