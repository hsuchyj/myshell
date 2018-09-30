#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "sh.h"

int sh( int argc, char **argv, char **envp )
{
  char *prompt = calloc(PROMPTMAX, sizeof(char));
  char *commandline = calloc(MAX_CANON, sizeof(char));
  char *command, *arg, *commandpath, *p, *pwd, *owd;
  char **args = calloc(MAXARGS, sizeof(char*));
  int uid, i, status, argsct, go = 1;
  struct passwd *password_entry;
  char *homedir;
  struct pathelement *pathlist;

  char cwd[256];
  char* token;
  pid_t pid;
  int n;
  char* argArr[25];//*******************************************************************NEED TO CHANGE TO **
  uid = getuid();
  password_entry = getpwuid(uid);               /* get passwd info */
  homedir = password_entry->pw_dir;		/* Home directory to start
						  out with*/

  if ( (pwd = getcwd(NULL, PATH_MAX+1)) == NULL )
  {
    perror("getcwd");
    exit(2);
  }
  owd = calloc(strlen(pwd) + 1, sizeof(char));
  memcpy(owd, pwd, strlen(pwd));
  prompt[0] = ' '; prompt[1] = '\0';

  /* Put PATH into a linked list */
  pathlist = get_path();
  //printf("%i",chdir("/home/hunter/Documents"));
  chdir("/home/hunter/Documents");
  //char* line;

  while ( go )
  {
    char* line = calloc(MAX_CANON, sizeof(char));
    char* cmd = malloc(120);
    /* print your prompt */
    //printf("%s",getcwd(cwd, PATH_MAX+1));
    getcwd(cwd, PATH_MAX+1);
    strcat(line, prompt);
    strcat(line, "[");
    strcat(line, cwd);
    strcat(line, "]>");
    printf("%s",line);

    /* get command line and process */
    fgets(commandline, sizeof(commandline), stdin);
    token = strtok(commandline, " ");
    int j = 0;
    while(token != NULL)
    {
      argv[j] = token;
      token = strtok(NULL, " ");
      j++;
    }
    command = argv[0];
    //printf("this is my command %s",command);
    command[strlen(command)-1] = '\0';
    /* check for each built in command and implement */
    int built = 0;
    char* builtins[] = {"exit", "which", "where", "cd","pwd", "list", "pid", "kill", "prompt", "printenv", "alias", "history", "setenv"};
    for(int i = 0; i < sizeof(builtins)/sizeof(builtins[0]); i++)
    {
      if(strcmp(builtins[i],command) == 0)
      {
          built = 1;
      }
    }
    if(built ==1)
    {
      sprintf(cmd, "/home/hunter/Downloads/proj_2/%s ",command);
      cmd[strlen(cmd)-1] = '\0';
      printf("this is length %lu \n", strlen(cmd));
      printf("this is cmd 2 %s \n",cmd);
      pid = fork();
      if(pid ==-1)
      {
        perror("fork error");
      }
      else if(pid > 0)
      {
        printf("this is child %s this is pid %i\n",cmd, pid);
        execve(cmd, argv, envp);
      }
      else if (pid == 0)
      {
        printf("this is parent %s this is pid %i\n",cmd, pid);
        //printf("Return not expected. Must be an execve error.n\n");
      }
      printf("%s\n",strerror(errno));

    }
    /*
    if(which(command, pathlist)!= NULL)
    {
      printf("%s",which(command, pathlist));
    }

    printf("%s",command);
    */
     /*  else  program to exec */
    //{
       /* find it */
       /* do fork(), execve() and waitpid() */

      //else
      //  fprintf(stderr, "%s: Command not found.\n", args[0]);
  //}
  //free(cmd);
  free(line);
  }
  return 0;
} /* sh() */

char *which(char *command, struct pathelement *pathlist )
{
   /* loop through pathlist until finding command and return it.  Return
   NULL when not found. */
   char* cmd = malloc(sizeof(char*));
   char* commandVal = NULL;
   //printf("%s", pathlist->element);
   while (pathlist)
   {
     strcpy(cmd, pathlist->element);
     strcat(cmd, "/");
     //command[strlen(command)-1] = '\0';
     strcat(cmd, command);
     //printf("%s", cmd);
     //printf("%i", access("/usr/bin/cd.sh", F_OK));
     cmd[strlen(cmd)-1] = '\0';
     //printf("\n");
     //printf("%lu", strlen(cmd));
     //printf("\n");
     //printf("%lu", strlen("/usr/bin/cd.sh"));
     //printf("\n");
     //access(cmd, F_OK);
     //struct stat s;
     /*
     if(strcmp(cmd, "/usr/bin/cd.sh") == 0)
     {
       //printf("found it");
     }
     */
     if(access(cmd, F_OK) == 0)
     {
        //printf("%i",access(cmd, F_OK));
        commandVal = command;
      }
     pathlist = pathlist->next;
   }

   return commandVal;
   /*
   char* commandVal = NULL;
   struct pathelement* temp = pathlist->next;
   while(temp != NULL)
   {
    char* testStr = malloc(sizeof(char*));
     //printf("%s",temp->element);
     strcat(testStr,temp->element);
     strcat(testStr,"/");
     strcat(testStr, command);
     if(access(testStr, F_OK) == 0)
     {
       commandVal = command;
     }
     temp = temp->next;
     free(testStr);
   }
   return commandVal;
   */

} /* which() */


char *where(char *command, struct pathelement *pathlist )
{
  /* similarly loop through finding all locations of command */
} /* where() */

void list ( char *dir )
{
  /* see man page for opendir() and readdir() and print out filenames for
  the directory passed */
} /* list() */
