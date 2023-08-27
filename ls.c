#include "shell.h"

void perms(int perm[][3], struct stat *test) {
  if ((test->st_mode & S_IRUSR) != 0)
    perm[0][0] = 1;
  else
    perm[0][0] = 0;
  if ((test->st_mode & S_IWUSR) != 0)
    perm[0][1] = 1;
  else
    perm[0][1] = 0;
  if ((test->st_mode & S_IXUSR) != 0)
    perm[0][2] = 1;
  else
    perm[0][2] = 0;
  if ((test->st_mode & S_IRGRP) != 0)
    perm[1][0] = 1;
  else
    perm[1][0] = 0;
  if ((test->st_mode & S_IWGRP) != 0)
    perm[1][1] = 1;
  else
    perm[1][1] = 0;
  if ((test->st_mode & S_IXGRP) != 0)
    perm[1][2] = 1;
  else
    perm[1][2] = 0;
  if ((test->st_mode & S_IROTH) != 0)
    perm[2][0] = 1;
  else
    perm[2][0] = 0;
  if ((test->st_mode & S_IWOTH) != 0)
    perm[2][1] = 1;
  else
    perm[2][1] = 0;
  if ((test->st_mode & S_IXOTH) != 0)
    perm[2][2] = 1;
  else
    perm[2][2] = 0;
  return;
}

void ls() {
  extern char *optarg;
  extern int optind;
  int c, flag_long = 0, flag_hidden = 0, flag_path = 0;
  optind = 0;
  while ((c = getopt(argcount, arg, "al")) != -1) {
    switch (c) {
      case 'l':
        flag_long = 1;
        break;
      case 'a':
        flag_hidden = 1;
        break;
      case '?':
        printf("Not a valid flag\n");
        break;
    }
  }
  char path[4096];
  if (optind < argcount) flag_path = 1;
  DIR *p;
  if (flag_path == 1) {
    p = opendir(arg[optind]);
    catch;
  } else {
    getcwd(path, 4096);
    p = opendir(path);
    catch;
  }
  struct dirent *file;
  while ((file = readdir(p))) {
    int print = 1;
    if (flag_hidden == 0 && file->d_name[0] == '.') print = 0;
    if (print == 1) {
      if (flag_long == 1) {
        struct stat details;
        char tmp[4096];
        strcpy(tmp, (flag_path == 1) ? arg[optind] : path);
        int len = strlen(tmp);
        tmp[len] = '/';
        tmp[len + 1] = '\0';
        strcat(tmp, file->d_name);
        lstat(tmp, &details);
        int perm[3][3];
        perms(perm, &details);
        char user[4096];
        get_user(details.st_uid, user);
        char group[4096];
        get_user(details.st_gid, group);
        char time[256];
        ctime_r(&details.st_mtime, time);
        int length = strlen(time);
        int i;
        for (i = 0; time[i] != '\n'; ++i)
          ;
        time[i] = '\0';
        char type = '-';
        if (S_ISDIR(details.st_mode) != 0)
          type = 'd';
        else if (S_ISLNK(details.st_mode) != 0)
          type = 'l';
        printf("%c%c%c%c%c%c%c%c%c%c %3lu %12s %12s %10ld %s %s\n", type,
               (perm[0][0] == 0) ? '-' : 'r', (perm[0][1] == 0) ? '-' : 'w',
               (perm[0][2] == 0) ? '-' : 'x', (perm[1][0] == 0) ? '-' : 'r',
               (perm[1][1] == 0) ? '-' : 'w', (perm[1][2] == 0) ? '-' : 'x',
               (perm[2][0] == 0) ? '-' : 'r', (perm[2][1] == 0) ? '-' : 'w',
               (perm[2][2] == 0) ? '-' : 'x', details.st_nlink, user, group,
               details.st_size, time, file->d_name);
      } else
        printf("%s ", file->d_name);
    }
  }
  printf("\n");
  optind = 1;
  return;
}
