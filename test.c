#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

int main(int argc, char **argv, char **envp)
{

  pid_t pid;
  argv[1] = "/home/hunter/Downloads/";
  if ((pid = fork()) ==-1)
    perror("fork error");
  else if (pid == 0)
  {
    execve("/home/hunter/Downloads/proj_2/cd", argv, envp);
    printf("Return not expected. Must be an execve error.n");
  }
}
