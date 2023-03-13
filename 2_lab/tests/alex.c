#include <stdio.h>

int main(int argc, char **argv, char **envp, char **argve) {
  printf("argc: %d\n", argc);
  for (int i = 0; i < argc; i++) {
    printf("argv[%d]: %s", i, argv[i]);
  }
}
