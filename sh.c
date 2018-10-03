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
#include <ctype.h>
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
  //char* argArr1[];

  int histLength = 25;
  char** history = malloc(100 * sizeof(char*));//*******************************************************************NEED TO CHANGE TO **
  for (i = 0; i < histLength; ++i)
  {
    history[i] = (char *)malloc(sizeof(char*));
  }
  for (i = 0; i < histLength; ++i)
  {
    history[i] = '\0';
  }
  char* prefix = malloc(sizeof(char*));
  int histIndex = 0;
  int numCommands = 0;

  prefix = "";
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
  char* line = calloc(MAX_CANON, sizeof(char));
  char* cmd = malloc(128);
  //char*cwd = malloc(sizeof(char*));
  while( go )
  {
    char** argArr = malloc(100 * sizeof(char*));//*******************************************************************NEED TO CHANGE TO **
    for (i = 0; i < 10; ++i)
    {
      argArr[i] = (char *)malloc(sizeof(char*));
    }
    for (i = 0; i < 10; ++i)
    {
      argArr[i] = '\0';
    }
    argArr[9] = NULL;
    //prefix = "";
    /* print your prompt */
    //printf("%s",getcwd(cwd, PATH_MAX+1));
    getcwd(cwd, PATH_MAX+1);
    sprintf(line,"%s %s[%s]>",prefix,prompt,cwd);
    printf("%s",line);

    /* get command line and process */
    fgets(commandline, 200, stdin);

    //adds space to end of command line
    commandline[strlen(commandline) - 1] = ' ';
    commandline[strlen(commandline)] = '\0';
    int k = 0;
    int len;
    char * token = strtok(commandline, " ");
    while(token != NULL)
    {
      //printf("%s\n", argArr[k]);
      argArr[k++] = token;
      token = strtok(NULL, " ");
      //token = strtok(NULL, " ");
    }
    len = k;
    printf("this is len %i\n", len);
    command = argArr[0];
    if(command != NULL)
    {
    //printf("this is before %s ind %i\n", history[0], histIndex);
    history[histIndex] = &command;
    //printf("this is after %s\n", history[0]);
    //printf("this is hist %s\n", history[histIndex]);
    numCommands++;

    //printf("this is argument 0 %s 1 %s len %lu\n", argArr[0], argArr[1], strlen(argArr[1]));
    //command[strlen(command)-1] = '\0';
    //checking for wildcard *
    int wildcard = 0;
    char* wild;
    int checkBuilt = 0;
    //k is length of array or num of args
    //printf("this is size %lu\n", (sizeof(argArr) / sizeof(argArr[0])));
    for(int i = 0; i < k; i++)
    {
      //printf("this is the stringer %s\n",argArr[i]);
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
      //wild[strlen(wild)-1] = '\0';
      csource = glob(wild, 0, NULL, &paths);

      if (csource == 0)
      {
        for (p = paths.gl_pathv; *p != NULL; ++p)
        {
          puts(*p);
          printf("%s\n",*p);
        }

        globfree(&paths);
        checkBuilt = 1;
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
          checkBuilt = 1;
          printf("Executing built in %s\n", built);
      }
    }
    //absolute paths
    if(checkBuilt == 0)
    {
      char* whichExe = malloc(sizeof(char*));
      char first = argArr[0][0];
      //argArr[0][strlen(argArr[0])-1] = '\0';
      //printf("%lu\n", strlen(argArr[1]));
      //if(first == '/' || first == '.')
      //{
        //printf("this is access %i\n", access(argArr[0],F_OK));
        //printf("this is len %lu\n", strlen(argArr[0]));
        //printf("%s\n",strerror(errno));

        if(first == '/' || first == '.')
        {
          //execve(argArr[0], argArr,  envp);
          whichExe = argArr[0];
        }
        else
        {
          if(which(command, pathlist)!= NULL)
          {
            printf("which is working %s\n",which(command, pathlist));
            //execve(which(command, pathlist), argArr,  envp);
            //for ls
            if(access(which(command, pathlist),F_OK) == 0)
            {
              whichExe = which(command, pathlist);
            }
            else
            {
              printf("does not exit\n");
            }
          }
        }
        //printf("this is exe  %s\n", whichExe);
        //if(access(which(command, pathlist),F_OK) == 0)
        //{
          //printf("this is exe %i\n", execve(argArr[0],argv, envp));
          pid = fork();
          //printf("l l %lu\n",strlen(argArr[1]));
          if(pid ==-1)
          {
            perror("fork error");
          }
          else if(pid > 0)
          {
            int status;
            printf("this is parent this is pid %i\n", pid);
            waitpid(pid,&status, 0);
          }
          else if (pid == 0)
          {
            printf("this is child this is pid %i\n", pid);
            //absolute path or ls
            execve(whichExe, argArr,  envp);
          }
            //printf("Return not expected. Must be an execve error.n\n");
          //execve(argArr[0],argv, envp);
        //}
      //}
      printf("%s\n",strerror(errno));
      free(whichExe);
    }
    //printf("%lu\n", strlen(built));
    if(strcmp(built,"cd") == 0)
    {
      //printf("build is cd\n");
      //argArr[1][strlen(argArr[1])-1] = '\0';
      //printf("%lu\n", strlen(argArr[1]));
      if(argArr[1] != NULL)
      {
        char first = argArr[1][0];
      if(first == '/')
      {
        prev = getcwd(prev, PATH_MAX+1);
        chdir(argArr[1]);
      }
      else if(strcmp(argArr[1],"-") == 0)
      {
        chdir(prev);
      }
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
    else if(strcmp(built,"history")==0)
    {
      int num = 10;
      if(numCommands < 10)
      {
        num = numCommands;
        printf("shrink num %i\n", num);
      }
      if(argArr[1] != NULL)
      {
        if(isdigit(atoi(argArr[1])) > 0)
        {
          num = atoi(argArr[1]);
        }
      }
      int i = 0;
      int tempInd = histIndex;
      for(int i = 0; i < len; i++)
      {
        printf("name uno %s\n", history[i]);
      }
      while(i < num)
      {
        printf("name %s\n", history[tempInd]);
        tempInd--;
        i++;

      }
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
          //argArr[i][strlen(argArr[i])-1] = '\0';
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
    else if (strcmp(built,"pid") == 0)
    {
      printf("%i\n",getpid());
    }
    else if (strcmp(built,"printenv") == 0)
    {
      if(argArr[2] != NULL)
      {
        printf("Too many args\n");
      }
      else if(isspace(argArr[1][0]) > 0)
      {
        for(int i = 0; i < sizeof(envp)/sizeof(envp[0]);i++)
        {
          printf("%s\n", envp[i]);
        }
      }
      else
      {
        char* env = getenv(argArr[1]);
        printf("%s\n",env);
        //printf("%s\n",getenv(argArr[1]));
      }
    }
    else if (strcmp(built,"prompt") == 0)
    {
      if(isspace(argArr[1][0]) > 0)
      {
        printf("input prompt prefix: ");
        fgets(prefix, 200, stdin);
        prefix[strlen(prefix)-1] = '\0';
      }
      else
      {
        prefix = argArr[1];
        prefix[strlen(prefix)-1] = '\0';
      }
    }
    else if (strcmp(built,"pwd") == 0)
    {
      printf("%s\n",cwd);
    }
    else if(strcmp(built,"where") == 0)
    {
      where(command,pathlist);
    }
    else if (strcmp(built,"which") == 0)
    {
      //printf("%s\n" , argArr[2]);
      for(int i = 1; i < len; i++)
      {
        printf("suze iz %i\n", len);
        if(argArr[i] != NULL)
        {
          printf("dis length %lu\n", strlen(argArr[i]));
          //argArr[i][strlen(argArr[i]) - 1] = '\0';
          printf("jes %s\n",argArr[i]);
          char* args = malloc(500);
          args = which(argArr[i],pathlist);
          printf("%s\n", args);
          free(args);
        }
      }
    }
    histIndex++;
    /*
    else if (strcmp(built,"setenv") == 0)
    {
      if(argArr[3] != NULL)
      {
        printf("Too many args\n");
      }
      else if(isspace(argArr[1][0]) > 0)
      {
        for(int i = 0; i < sizeof(envp)/sizeof(envp[0]);i++)
        {
          printf("%s\n", envp[i]);
        }
      }
      else
      {
        printf("%s\n",getenv(argArr[1]));
      }
    }
    */
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

      //free(cmd);
      //free(cwd);
      //free(line);
      //free(argArr);
      for (i = 0; i < 10; ++i)
      {
        free(argArr[i]);
      }
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
  //free(line);
    }
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
   char* cmd = malloc(360);
   char* commandVal = malloc(360);
   //printf("%s", pathlist->element);
   while (pathlist)
   {
     strcpy(cmd, pathlist->element);
     strcat(cmd, "/");
     //command[strlen(command)-1] = '\0';
     strcat(cmd, command);
     //printf("%s\n", cmd);
     //printf("%i", access("/usr/bin/cd.sh", F_OK));
     //cmd[strlen(cmd)-1] = '\0';
     //printf("\n");
     //printf("%lu\n", strlen(cmd));
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
        printf("%i\n",access(cmd, F_OK));
        commandVal = cmd;
        break;
      }
     pathlist = pathlist->next;
   }

   //return commandVal;
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
   return commandVal;
   //free(cmd);
   //free(commandVal);
} /* which() */


void *where(char *command, struct pathelement *pathlist )
{
  /* similarly loop through finding all locations of command */
  char* cmd = malloc(sizeof(char*));
  char* commandVal = NULL;
  while (pathlist)
  {
    strcpy(cmd, pathlist->element);
    strcat(cmd, "/");
    strcat(cmd, command);
    cmd[strlen(cmd)-1] = '\0';
    if(access(cmd, F_OK) == 0)
    {
       printf("%s\n",cmd);
    }
    pathlist = pathlist->next;
  }

} /* where() */


void list ( char *dir )
{
  /* see man page for opendir() and readdir() and print out filenames for
  the directory passed */
} /* list() */
