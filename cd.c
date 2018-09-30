#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

int main(int argc, char **argv, char **envp)
{
  printf("This is an argument %s\n", argv[1]);
  if(strcmp(argv[1],"") == 0)
  {
    printf("empty");
    chdir("/home/");
  }
  else if(strcmp(argv[1],"-") == 0)
  {
    printf("dash");
    //go to previous directoory
  }
  else
  {
    printf("change this dir\n");
    chdir(argv[1]);
  }
  //printf("wtf");
  //exit();
}
