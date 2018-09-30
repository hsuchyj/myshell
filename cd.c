#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

int main(int argc, char **argv, char **envp)
{
  printf("This is an argument");
  if(strcmp(argv[1],"") == 0)
  {
    chdir("/home/");
  }
  else if (strcmp(argv[1],"-") == 0)
  {
    //go to previous directoory
  }
  else
  {
    chdir(argv[1]);
  }
}
