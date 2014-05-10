#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
  int a;
  int* b;
  a = 123;
  b = (int*) malloc(sizeof(int));
  *b = 456;
  a = 789 * a;
  a = 321 + *b;
  b = &a;
}
