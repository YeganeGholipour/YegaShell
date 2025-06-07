#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "env_utils.h"

void cleanup(char **envp) {
  free_variable_table();
  for (int i = 0; envp[i]; i++)
    free(envp[i]);

  free(envp);
}

void test_add_variable() {
  const char *key = "HOME";
  const char *value = "/home/yegane";
  Variable *vp = add_variable(key, value, 1);

  assert(vp != NULL);
  assert(strcmp(vp->key, "HOME") == 0);
  assert(strcmp(vp->value, "/home/yegane") == 0);

  free_variable_table();
  printf("test_add_variable passes.\n");
}

void test_remove_variable_exist() {
  Variable *vp = add_variable("HOME", "/home/yegane", 1);
  int status = remove_variable("HOME");

  assert(status == 0);
  assert(lookup("HOME") == NULL);

  printf("test_remove_variable_exist passes.\n");
}

void test_remove_variable_does_not_exist() {
  int status = remove_variable("HOME");

  assert(status == -1);

  printf("test_remove_variable_does_not_exist passes.\n");
}

void test_valid_command_path() {
  char *path = get_full_path("ls");
  assert(path != NULL);
  assert(strcmp(path, "/usr/bin/ls") == 0);
  free(path);

  printf("test_valid_command_path passes.\n");
}

void test_invalid_command_path() {
  char *path = get_full_path("bb");
  assert(path == NULL);

  printf("test_invalid_command_path passes.\n");
}

void test_build_envp() {
  add_variable("HOME", "/home/yegane", 1);
  add_variable("TERM", "xterm", 1);

  char **envp = build_envp();

  assert(envp != NULL);
  assert(strcmp(envp[0], "HOME=/home/yegane") == 0);
  assert(strcmp(envp[1], "TERM=xterm") == 0);

  cleanup(envp);
  
  printf("test_build_envp passes.\n");
}


int main(void) {
  test_add_variable();
  test_remove_variable_exist();
  // test_remove_variable_does_not_exist();
  test_valid_command_path();
  test_invalid_command_path();
  test_build_envp();

  printf("All tests passed!\n");
  return 0;
}