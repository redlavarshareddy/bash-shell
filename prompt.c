#include "shell.h"
char *commands[256];
char home[4096];
char *arg[32767];
char argcount;
int foreground_proc = 0;
void handle_SIGINT(int signum) {
  // Reset first
  if (foreground_proc != 0) kill(foreground_proc, 9);
  signal(SIGINT, handle_SIGINT);
  return;
}

void handle_SIGTSTP(int signum) {
  // Reset first
  if (foreground_proc != 0)
    if (kill(foreground_proc, SIGSTOP) != -1) {
      proc_node *table_end = table_start;
      int i = 1;
      while (table_end != NULL && table_end->next != NULL)
        table_end = table_end->next, ++i;
      // Make an entry into the process table
      if (table_end == NULL) {
        table_start = malloc(sizeof(proc_node));
        table_end = table_start;
        table_start->pid = foreground_proc;
        table_start->status = 1;
        table_start->next = NULL;
        table_end = table_start;
      } else {
        table_end->next = malloc(sizeof(proc_node));
        table_end = table_end->next;
        table_end->next = NULL;
        table_end->status = 1;
        table_end->pid = foreground_proc;
      }
      char pcb[128];
      sprintf(pcb, "/proc/%d/cmdline", foreground_proc);
      int fd = open(pcb, O_RDONLY);
      char cmd[128];
      read(fd, cmd, 128);
      close(fd);
      printf("[%d]\t%s\t%s\t[%d]\n", i, "Stopped", cmd, foreground_proc);
    }
  signal(SIGTSTP, handle_SIGTSTP);
  return;
}

int main(void) {
  int run = 0;
  signal(SIGINT, handle_SIGINT);
  signal(SIGTSTP, handle_SIGTSTP);
  // Set the home variable
  getcwd(home, 4096);
  setpgid(getpid(), getpid());
  while (run == 0) {
    prompt();
    free_args();
    _is_background = 0;
    read_cmd();
    for (int i = 0; commands[i] != NULL; ++i) {
      char *piped[256];
      int pipes[256][2];
      int j = 0;
      for (char *s = strtok(commands[i], "|"); s != NULL;
           ++j, s = strtok(NULL, "|")) {
        piped[j] = malloc(strlen(s) + 1);
        strcpy(piped[j], s);
      }
      piped[j] = NULL;
      if (j > 1) {
        // First pipe
        pipe(pipes[0]);
        free_args();
        int leader = fork();
        if (leader == 0) {
          // close(1);
          dup2(pipes[0][1], 1);  // Redirect my output to the pipe
          close(pipes[0][0]);
          interpret(piped[0]);
          if (argcount == 0) continue;
          if (strncmp(arg[0], "cd", 2) == 0) {
            cd();
          } else if (strcmp(arg[0], "ls\0") == 0) {
            ls();
          } else if (strcmp(arg[0], "pwd\0") == 0) {
            pwd();
          } else if (strcmp(arg[0], "echo\0") == 0) {
            echo();
          } else if (strcmp(arg[0], "pinfo\0") == 0) {
            pinfo();
          } else if (strcmp(arg[0], "remindme\0") == 0) {
            remindme();
          } else if (strcmp(arg[0], "clock\0") == 0) {
            clock_wrapper();
          } else if (strcmp(arg[0], "setenv\0") == 0) {
            Mysetenv();
          } else if (strcmp(arg[0], "unsetenv\0") == 0) {
            Myunsetenv();
          } else {
            fflush(stdout);
            if (execvp(arg[0], arg) == -1) {
              printf("Wrong Command\n");
              exit(0);
            }
          }
          // close(1);
          exit(0);
        }
        close(pipes[0][1]);  // Close unused fd (Read End)
        setpgid(leader, leader);
        // close(pipes[0][1]);
        for (int k = 1; k < j - 1; ++k) {
          pipe(pipes[k]);
          int middling = fork();
          if (middling == 0) {
            dup2(pipes[k][1], 1);  // Redirect my output to the pipe
            close(pipes[k][0]);
            dup2(pipes[k - 1][0], 0);  // Redirect my input to pipe
            close(pipes[k - 1][1]);
            interpret(piped[k]);
            if (argcount == 0) continue;
            if (strncmp(arg[0], "cd", 2) == 0) {
              cd();
            } else if (strcmp(arg[0], "ls\0") == 0) {
              ls();
            } else if (strcmp(arg[0], "pwd\0") == 0) {
              pwd();
            } else if (strcmp(arg[0], "echo\0") == 0) {
              echo();
            } else if (strcmp(arg[0], "pinfo\0") == 0) {
              pinfo();
            } else if (strcmp(arg[0], "remindme\0") == 0) {
              remindme();
            } else if (strcmp(arg[0], "clock\0") == 0) {
              clock_wrapper();
            } else {
              // fflush(stdout);
              if (execvp(arg[0], arg) == -1) {
                printf("Wrong Command\n");
                exit(0);
              }
            }
            // close(pipes[k - 1][0]);  // Close unused fd (Read End)
            // close(pipes[k][1]);      // close unused fd (Write End)
            exit(0);
          }
          close(pipes[k - 1][0]);  // Close unused fd (Read End)
          close(pipes[k][1]);      // close unused fd (Write End)
          setpgid(middling, leader);
        }

        int laggard = fork();  // last of the real ones
        free_args();
        if (laggard == 0) {
          dup2(pipes[j - 2][0], 0);  // Redirect my input to pipe
          close(pipes[j - 2][1]);
          interpret(piped[j - 1]);
          if (argcount == 0) continue;
          if (strncmp(arg[0], "cd", 2) == 0) {
            cd();
          } else if (strcmp(arg[0], "ls\0") == 0) {
            ls();
          } else if (strcmp(arg[0], "pwd\0") == 0) {
            pwd();
          } else if (strcmp(arg[0], "echo\0") == 0) {
            echo();
          } else if (strcmp(arg[0], "pinfo\0") == 0) {
            pinfo();
          } else if (strcmp(arg[0], "remindme\0") == 0) {
            remindme();
          } else if (strcmp(arg[0], "clock\0") == 0) {
            clock_wrapper();
          } else if (strcmp(arg[0], "setenv\0") == 0) {
            Mysetenv();
          } else if (strcmp(arg[0], "unsetenv\0") == 0) {
            Myunsetenv();
          } else {
            if (execvp(arg[0], arg) == -1) {
              printf("Wrong Command\n");
              exit(0);
            }
          }
          // close(pipes[j - 2][1]);  // close unused fd (Write End)
          exit(0);
        }
        close(pipes[j - 2][0]);  // close unused fd (Write End)
        setpgid(laggard, leader);
        // close(pipes[0][0]);
        while (waitpid(-1, NULL, 0) != -1)
          ;

        errno = 0;
      } else {
        free_args();
        // Save state of fd table
        int out = dup(1), in = dup(0);
        interpret(commands[i]);
        if (argcount == 0) continue;
        if (strncmp(arg[0], "cd", 2) == 0) {
          cd();
        } else if (strcmp(arg[0], "ls\0") == 0) {
          ls();
        } else if (strcmp(arg[0], "pwd\0") == 0) {
          pwd();
        } else if (strcmp(arg[0], "echo\0") == 0) {
          echo();
        } else if (strcmp(arg[0], "pinfo\0") == 0) {
          pinfo();
        } else if (strcmp(arg[0], "remindme\0") == 0) {
          remindme();
        } else if (strcmp(arg[0], "clock\0") == 0) {
          clock_wrapper();
        } else if (strcmp(arg[0], "setenv\0") == 0) {
          Mysetenv();
        } else if (strcmp(arg[0], "unsetenv\0") == 0) {
          Myunsetenv();
        } else if (strcmp(arg[0], "jobs\0") == 0) {
          jobs();
        } else if (strcmp(arg[0], "fg\0") == 0) {
          fg();
        } else if (strcmp(arg[0], "bg\0") == 0) {
          bg();
        } else if (strcmp(arg[0], "kjob\0") == 0) {
          kjob();
        } else if (strcmp(arg[0], "overkill\0") == 0) {
          overkill();
        } else if (strcmp(arg[0], "quit\0") == 0) {
          return 0;
        } else {
          execute_cmd();
        }
        fflush(stdout);
        // Restore state of fd table
        dup2(out, 1);
        dup2(in, 0);
      }
    }
  }
  return 0;
}
