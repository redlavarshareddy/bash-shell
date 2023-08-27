#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
int main() {
  char s[202];
  int i;

  printf("What to execute?\n");
  scanf("%s", s);
  int x = open("out.txt", O_RDWR);
  dup2(x, 1);
  printf("test\n");
  /*
    This is just a sample.
    You may need to use different version of the exec command in your coming
    assignment.
  */
  execlp(s, s, (char *)NULL);

  // The process would never reach this part if exec worked properly
  printf("This line will be printed iff there was an error in exec\n");
  return 0;
}