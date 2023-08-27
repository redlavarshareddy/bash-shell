#include "shell.h"

void remindme() {
  if (argcount < 3) {
    printf("Invalid Usage\n");
    return;
  }
  int time = atoi(arg[1]);
  char msg[4096];
  int t = 0;
  for (int i = 2; i < argcount; ++i) {
    for (int j = 0; arg[i][j] != '\0'; ++j)
      if (arg[i][j] != '"') msg[t++] = arg[i][j];
    msg[t++] = ' ';
  }
  msg[t] = '\0';
  int pid = fork();
  if (pid == 0) {
    sleep(time);
    printf("\nReminder : %s\n", msg);
  }
  return;
}

void clock_rtc() {
  int rtc = open("/proc/driver/rtc", O_RDONLY);
  char buffer[1024];
  read(rtc, buffer, 1024);
  strtok(buffer, ":");
  char *time_gmt_tmp = strtok(NULL, "\n");
  char time_gmt[256];
  strcpy(time_gmt, time_gmt_tmp + 1);
  strtok(NULL, ":");  // Cycle through label
  char *date_gmt_tmp = strtok(NULL, "\n");
  char date_gmt[256];
  strcpy(date_gmt, date_gmt_tmp + 1);
  char buffer_time[512];
  strcpy(buffer_time, date_gmt);
  int len = strlen(buffer_time);
  buffer_time[len] = ' ';
  buffer_time[len + 1] = '\0';
  strcat(buffer_time, time_gmt);
  printf("%s\n", buffer_time);
  return;
}

void clock_wrapper() {
  extern char *optarg;
  extern int optind;
  int c;
  int time = 1, step = 1;
  optind = 0;
  while ((c = getopt(argcount, arg, "n:t:")) != -1) {
    switch (c) {
      case 'n':
        time = atoi(optarg);
        break;
      case 't':
        step = atoi(optarg);
        break;
      case '?':
        printf("Invalid Flag\n");
        break;
    }
  }
  for (int i = 0; i < time; i += step) {
    clock_rtc();
    sleep(step);
  }
  return;
}