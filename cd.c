#include <unistd.h>
#include <stdio.h>
void cd(char* dir[])
{
  printf("This is an argument %s", dir[1]);
  if(dir[1] == "")
  {
    chdir("/home/");
  }
  else if (dir[1] == "-")
  {
    //go to previous directoory
  }
  else
  {
    chdir(dir[1]);
  }
}
