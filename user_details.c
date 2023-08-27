#include "shell.h"

void get_user(int uid, char *uname) {
  int list = open("/etc/passwd", O_RDONLY);
  catch;
  char buffer[MAX_READ + 1];
  ssize_t sz = read(list, buffer, MAX_READ);
  catch;
  for (char *s = strtok(buffer, "\n"); s != NULL; s = strtok(NULL, "\n")) {
    char *name = strtok(NULL, ":");
    strtok(NULL, ":");  // Cycle through the password
    char *this_uid = strtok(NULL, ":");
    if (this_uid != NULL && atoi(this_uid) == uid) {
      strtok(NULL, ":");  // Cycle through Group ID
      strtok(NULL, ":");  // Cycle through extra info
      // char *home = strtok(NULL, ":");
      strcpy(uname, name);
      // strcpy(instance->home, home);
      break;
    }
  }
  return;
}