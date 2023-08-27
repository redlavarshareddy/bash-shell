#include "shell.h"

proc_node *table_start = NULL;
proc_node *table_end = NULL;
// Signal Handler called when a child process terminates
void sig_child_process_terminated(int sig) {
  pid_t pid;
  int status;
  while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
    if (WIFEXITED(status)) {
      proc_node *reader = table_start;
      if (reader->pid == pid) {
        table_start = reader->next;
        free(reader);
        continue;
      }
      while (reader->next != NULL) {
        proc_node *next_reader = reader->next;
        if (next_reader->pid == pid) {
          reader->next = next_reader->next;
          free(next_reader);
          break;
        }
        reader = reader->next;
      }
      printf("Process with pid %d exited normally\n", pid);
    } else if (WIFSTOPPED(status)) {
      proc_node *reader = table_start;
      while (reader != NULL) {
        if (reader->pid == pid) {
          reader->status = 1;
          break;
        }
        reader = reader->next;
      }
    } else {
      proc_node *reader = table_start;
      if (reader->pid == pid) {
        table_start = reader->next;
        free(reader);
        continue;
      } else {
        while (reader != NULL) {
          proc_node *next_reader = reader->next;
          if (next_reader->pid == pid) {
            reader->next = next_reader->next;
            free(next_reader);
            break;
          }
          reader = reader->next;
        }
      }
      printf("Process with pid %d exited with status %d\n", pid, status);
    }
  }
  if (errno == ECHILD) signal(SIGCHLD, SIG_DFL), errno = 0;
  return;
}

void execute_cmd() {
  int i = 0;
  if (_is_background == 0) {
    pid_t pid = fork();
    if (pid == 0) {
      if (execvp(arg[0], arg) == -1) {
        printf("Wrong Command\n");
        exit(0);
      }
    } else {
      foreground_proc = pid;
      waitpid(pid, NULL, WUNTRACED);
      foreground_proc = 0;
    }
  } else {
    pid_t pid = fork();
    if (pid == 0) {
      // signal(SIGINT, SIG_DFL);
      // signal(SIGQUIT, SIG_DFL);
      // signal(SIGTTIN, SIG_DFL);
      // signal(SIGTTOU, SIG_DFL);
      // signal(SIGCHLD, SIG_DFL);
      setpgid(getpid(), pid);
      if (execvp(arg[0], arg) == -1) {
        printf("Wrong Command\n");
        exit(0);
      }
    } else {
      foreground_proc = 0;
      signal(SIGCHLD, sig_child_process_terminated);
      table_end = table_start;
      while (table_end != NULL && table_end->next != NULL)
        table_end = table_end->next;
      // Make an entry into the process table
      if (table_end == NULL) {
        table_start = malloc(sizeof(proc_node));
        table_end = table_start;
        table_start->pid = pid;
        table_start->status = 0;
        table_start->next = NULL;
        table_end = table_start;
      } else {
        table_end->next = malloc(sizeof(proc_node));
        table_end = table_end->next;
        table_end->next = NULL;
        table_end->status = 0;
        table_end->pid = pid;
      }
      printf("Process with pid %d running in background\n", pid);
    }
  }
  return;
}

void pinfo() {
  int pid;
  if (argcount == 1)
    pid = getpid();
  else
    pid = atoi(arg[1]);
  char proc_path[4096];
  sprintf(proc_path, "/proc/%d/stat", pid);
  int stat = open(proc_path, O_RDONLY);
  catch;
  char buffer[4096];
  read(stat, buffer, 4096);
  catch;
  char state;
  char comm[256];
  int ppid, pgrp, session, tty_nr, tpgid;
  unsigned int flags;
  unsigned long int minflt, cminflt, majflt, cmajflt, utime, vsize, stime;
  long int itrealvalue, cutime, cstime, priority, nice, num_threads;
  unsigned long long int starttime;
  sscanf(buffer,
         "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld "
         "%ld %ld %llu %lu",
         &pid, comm, &state, &ppid, &pgrp, &session, &tty_nr, &tpgid, &flags,
         &minflt, &cminflt, &majflt, &cmajflt, &utime, &stime, &cutime, &cstime,
         &priority, &nice, &num_threads, &itrealvalue, &starttime, &vsize);
  close(stat);
  sprintf(proc_path, "/proc/%d/exe", pid);
  char exe[4096];
  int len = readlink(proc_path, exe, 4096);
  exe[len] = '\0';
  int start = 0;
  int length = strlen(home);
  if (strncmp(exe, home, length) == 0) {
    exe[length - 1] = '~';
    start = length - 1;
  }
  printf(
      "pid -- %d\n\nProcess Status -- %c\nMemory - %lu\nExecutable Path - "
      "%s\n",
      pid, state, vsize, exe + start);
  return;
}

void jobs() {
  proc_node *Read = table_start;
  for (int i = 1; Read != NULL; ++i, Read = Read->next) {
    char pcb[128];
    sprintf(pcb, "/proc/%d/cmdline", Read->pid);
    int fd = open(pcb, O_RDONLY);
    char cmd[128];
    read(fd, cmd, 128);
    close(fd);
    printf("[%d]\t%s\t%s\t[%d]\n", i,
           (Read->status == 0) ? "Running" : "Stopped", cmd, Read->pid);
  }
}

void fg() {
  if (argcount != 2) {
    printf("Invalid usage\n");
    return;
  }
  int job = atoi(arg[1]);
  proc_node *reader = table_start;
  for (int i = 0; i < job - 1; ++i) {
    if (reader == NULL) {
      printf("No such job exists\n");
      return;
    }
    reader = reader->next;
  }
  if (reader == NULL) {
    printf("No such job exists\n");
    return;
  }
  int tmp = reader->pid;
  proc_node *reader2 = table_start;
  kill(tmp, SIGCONT);
  if (reader2->pid == reader->pid) {
    table_start = reader2->next;
    free(reader2);
  } else {
    while (reader2->next != NULL) {
      proc_node *next_reader = reader2->next;
      if (next_reader->pid == reader->pid) {
        reader2->next = next_reader->next;
        free(next_reader);
        break;
      }
      reader2 = reader2->next;
    }
  }
  signal(SIGINT, handle_SIGINT);
  signal(SIGTSTP, handle_SIGTSTP);
  foreground_proc = tmp;
  waitpid((pid_t)tmp, NULL, WUNTRACED);
  foreground_proc = 0;
  return;
}

void bg() {
  if (argcount != 2) {
    printf("Invalid usage\n");
    return;
  }
  int job = atoi(arg[1]);
  proc_node *reader = table_start;
  for (int i = 0; i < job - 1; ++i) {
    if (reader == NULL) {
      printf("No such job exists\n");
      return;
    }
    reader = reader->next;
  }
  if (reader == NULL) {
    printf("No such job exists\n");
    return;
  }
  printf("%d\n", reader->pid);
  if (kill(reader->pid, SIGCONT) != -1) reader->status = 0;
  return;
}

void kjob() {
  if (argcount != 3) {
    printf("Invalid usage\n");
    return;
  }
  int job = atoi(arg[1]);
  proc_node *reader = table_start;
  for (int i = 0; i < job - 1; ++i) {
    if (reader == NULL) {
      printf("No such job exists\n");
      return;
    }
    reader = reader->next;
  }
  if (reader == NULL) {
    printf("No such job exists\n");
    return;
  }
  int sig = atoi(arg[2]);
  kill(reader->pid, sig);
  return;
}

void overkill() {
  proc_node *reader = table_start;
  while (reader != NULL) {
    kill(reader->pid, 9);
    reader = reader->next;
  }
  return;
}