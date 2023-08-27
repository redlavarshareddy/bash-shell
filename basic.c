#include "shell.h"
int _is_background = 0;
int isOutputRedirected = 0;
int isInputRedirected = 0;
void free_args() {
  for (int i = 0; arg[i] != NULL; ++i) {
    free(arg[i]);
    arg[i] = NULL;
  }
}

void prompt() {
  char path[4096];
  getcwd(path, 4096);
  catch;
  char name[4096];
  get_user(getuid(), name);
  struct utsname details;
  uname(&details);
  catch;
  int start_path = 0;
  if (strncmp(home, path, strlen(home)) == 0) {
    start_path = strlen(home) - 1;
    path[start_path] = '~';
  }
  printf("<%s@%s:%s> ", name, details.nodename, path + start_path);
}

void read_cmd() {
  char buffer[131072];
  memset(buffer, 0, 131072);
  for (int i = 0; commands[i] != NULL; ++i) {
    free(commands[i]);
    commands[i] = NULL;
  }
  int i = 0;
  while ((buffer[i] = fgetc(stdin)) != EOF) {
    if (buffer[i] == '\n' && (i == 0 || buffer[i - 1] != '\\')) break;
    ++i;
  }
  if (i == 0) {
    argcount = 0;
    return;
  }
  i = 0;
  for (char *s = strtok(buffer, ";"); s != NULL && i < 32767;
       s = strtok(NULL, ";"), ++i) {
    commands[i] = malloc(strlen(s) + 1);
    strcpy(commands[i], s);
  }
  commands[i] = NULL;
  return;
}

void interpret(char *cmd) {
  free_args();
  int j = 0;
  char *op = NULL;  // Output file
  char *ip = NULL;  // Input file
  int append = 0;   // Should append = 1 or overwrite = 0
  for (char *s = strtok(cmd, WHITESPACE); s != NULL && j < 32767;
       s = strtok(NULL, WHITESPACE)) {
    char *new;
    switch (s[0]) {
      case '~':
        new = malloc((strlen(home) + strlen(s) + 3) * sizeof(char));
        strcpy(new, home);
        int len = strlen(home);
        if (s[1] != '/') {
          new[len] = '/';
          new[len + 1] = '\0';
        }
        strcat(new, s + 1);
        free(s);
        arg[j] = new;
        ++j;
        break;
      case '>':
        if (s[1] == '>') append = 1;
        s = strtok(NULL, WHITESPACE);
        if (s == NULL) {
          printf("Output filename missing\n");
          return;
        }
        op = malloc(strlen(s) * sizeof(char));
        strcpy(op, s);
        isOutputRedirected = 1;
        break;
      case '<':
        s = strtok(NULL, WHITESPACE);
        if (s == NULL) {
          printf("Input filename missing\n");
          return;
        }
        ip = malloc(strlen(s) * sizeof(char));
        strcpy(ip, s);
        isInputRedirected = 1;
        break;
      case '&':
        _is_background = 1;
        break;
      default:
        arg[j] = malloc(sizeof(char) * strlen(s));
        strcpy(arg[j], s);
        ++j;
        break;
    }
  }
  arg[j] = NULL;
  argcount = j;
  if (op != NULL) {
    int out;
    if (append == 0)
      out = open(op, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    else
      out = open(op, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    dup2(out, 1);
  }
  if (ip != NULL) {
    int in = open(ip, O_RDONLY);
    catch;
    close(0);
    dup2(in, 0);
  }
  return;
}