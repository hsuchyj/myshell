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
#include <sys/wait.h>
#include <glob.h>
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
  pid_t pid;
  int n;
  char** argArr = malloc(sizeof(char**));//*******************************************************************NEED TO CHANGE TO **
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
  //chdir("/home/hunter/Documents");
  //char* line;
  char *prev;
  signal(SIGINT, catchCtrlC);
  //sigignore(SIGSTOP);
  //sigignore(SIGTERM);
  while( go )
  {
    char* line = calloc(MAX_CANON, sizeof(char));
    char* cmd = malloc(128);
    char*cwd = malloc(128);
    /* print your prompt */
    //printf("%s",getcwd(cwd, PATH_MAX+1));
    getcwd(cwd, PATH_MAX+1);
    sprintf(line,"%s[%s]>",prompt,cwd);
    printf("%s",line);

    /* get command line and process */
    fgets(commandline, 200, stdin);
    char* token = strtok(commandline, " ");
    int k = 0;
    int len;
    while(token != NULL)
    {
      argArr[k] = token;
      token = strtok(NULL, " ");
      k++;
    }
    len = k;
    command = argArr[0];
    printf("this is argument 0 %s 1 %s k %i\n", argArr[0], argArr[1], k);
    //command[strlen(command)-1] = '\0';
    //checking for wildcard *
    int wildcard = 0;
    char* wild;
    //k is length of array or num of args
    //printf("this is size %lu\n", (sizeof(argArr) / sizeof(argArr[0])));
    for(int i = 0; i < k; i++)
    {
      printf("this is the stringer %s\n",argArr[i]);
      for(int j = 0; j < sizeof(argArr[i])/sizeof(argArr[i][0]); j++)
      {
          if(argArr[i][j] == '*')
          {
            wildcard = 1;
            wild = argArr[i];
          }
      }
    }

    if(wildcard == 1)
    {
      printf("this is the wild statement  %s", wild);
      glob_t  paths;
      int     csource;
      char    **p;
      wild[strlen(wild)-1] = '\0';
      csource = glob(wild, 0, NULL, &paths);

      if (csource == 0)
      {
        for (p = paths.gl_pathv; *p != NULL; ++p)
        {
          puts(*p);
          printf("%s\n",*p);
        }

        globfree(&paths);
      }
    }

    /* check for each built in command and implement */
    char* built = "";
    char* builtins[] = {"exit", "which", "where", "cd","pwd", "list", "pid", "kill", "prompt", "printenv", "alias", "history", "setenv"};
    for(int i = 0; i < sizeof(builtins)/sizeof(builtins[0]); i++)
    {
      if(strcmp(builtins[i],command) == 0)
      {
          built = builtins[i];
          printf("Executing built in %s\n", built);
      }
    }
    //printf("%lu\n", strlen(built));
    if(strcmp(built,"cd") == 0)
    {
      //printf("build is cd\n");
      char first = argArr[1][0];
      argArr[1][strlen(argArr[1])-1] = '\0';
      //printf("%lu\n", strlen(argArr[1]));
      if(first == '/')
      {
        prev = getcwd(prev, PATH_MAX+1);
        chdir(argArr[1]);
      }
      else if(strcmp(argArr[1],"-") == 0)
      {
        chdir(prev);
      }
      else
      {
        prev = getcwd(prev, PATH_MAX+1);
        chdir("/home/");
      }
    }
    else if(strcmp(built,"exit") == 0)
    {
      exit(0);
    }
    else if(strcmp(built,"list") == 0)
    {
      char first = argArr[1][0];
      if(first == '/')
      {
        DIR* d = malloc(sizeof(DIR*));
        struct dirent *dp = malloc(sizeof(struct dirent*));
        //for(int i = 1; i < sizeof(argArr); i++)
        int i = 1;
        while(argArr[i] != NULL)
        {
          argArr[i][strlen(argArr[i])-1] = '\0';
          d = opendir(argArr[i]);
          printf("\n");
          printf("%s:\n", argArr[i]);
          while ((dp=readdir(d)) != NULL)
          {
            printf("%s\n",dp->d_name);
          }
          closedir(d);
          i++;
          //free(dp);
          //free(d);
        }
      }
      else
      {
          DIR* d = opendir(cwd);
          struct dirent *dp;
          while ((dp=readdir(d)) != NULL)
          {
            printf("%s\n",dp->d_name);
          }
          free(d);
      }
    }
    //JUST FOR COMMANDS NOT ON BUILT IN LIST cat, ls, etc.
    //also has to use which()
    if(built =="cheese")
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
        int status;
        printf("this is parent %s this is pid %i\n",cmd, pid);
        waitpid(pid,&status, 0);
      }
      else if (pid == 0)
      {
        if(access(cmd, F_OK) == 0)
        {
          execve(cmd, argArr, envp);
        }
        printf("this is child %s this is pid %i\n",cmd, pid);
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
  //free(cwd);
  free(line);
  }
  return 0;
} /* sh() */

void catchCtrlC(int sig_num)
{
    signal(SIGINT, catchCtrlC);
}


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
