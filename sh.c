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
#include <pthread.h>
#include <utmpx.h>
#include "sh.h"
#include "ll.h"


//global linked list
node head = NULL;
node tail = malloc(sizeof(node));
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
  pthread_t watchUser;
  int watched = 0;

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
  char* prefix2= malloc(sizeof(char*));
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
  signal(SIGSTOP, catchCtrlC);
  signal(SIGTSTP, catchCtrlC);
  signal(SIGTERM, catchCtrlC);
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
    //printf("this is len %i\n", len);
    command = argArr[0];
    if(command != NULL)
    {
    //printf("this is before %s ind %i\n", history[0], histIndex);
    history[histIndex] = command;
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
    //check for wild
    if(wildcard == 1)
    {
      //printf("this is the wild statement  %s", wild);
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
    char* builtins[] = {"exit", "which", "where", "cd","pwd", "list", "pid", "kill", "prompt", "printenv", "alias", "history", "setenv","watchuser"};
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
      printf("%i\n",argArr[len-1] == "&");
    if(strcmp(argArr[len-1],"&") == 0)
    {
      printf("has a ampersand\n");
      char* whichExe = malloc(sizeof(char*));
      pthread_t thread1;
      int ret = 0;
      if(which(command, pathlist)!= NULL)
      {
        if(access(which(command, pathlist),F_OK) == 0)
        {
          whichExe = which(command, pathlist);
        }
        else
        {
          printf("does not exit\n");
        }
      }
      //pthread_create(&thread1, NULL, (void*) externalCommand(whichExe), NULL);
      strcat(whichExe, " &");
      whichExe[strlen(whichExe)-1] = '\0';
      system(whichExe);

    }
    else if(checkBuilt == 0)
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
        //background processes
        else
        {
          if(which(command, pathlist)!= NULL)
          {
            //printf("which is working %s\n",which(command, pathlist));
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
            //printf("this is parent this is pid %i\n", pid);
            waitpid(pid,&status, 0);
          }
          else if (pid == 0)
          {
            //printf("this is child this is pid %i\n", pid);
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
      if(len > 2)
      {
          printf("too many args\n");
      }
      else if(argArr[1] != NULL)
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
        //printf("shrink num %i\n", num);
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
        printf("%s\n", history[i]);
      }
      while(i < num)
      {
        //printf("name %s\n", history[tempInd]);
        tempInd--;
        i++;

      }
    }
    else if(strcmp(built,"kill") == 0)
    {
        printf("killing process\n");
        kill( getpid(), SIGTERM);
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
    else if(strcmp(built,"alias") == 0)
    {
      printf("alias table\n");
    }
    else if (strcmp(built,"pid") == 0)
    {
      printf("%i\n",getpid());
    }
    else if (strcmp(built,"printenv") == 0)
    {
      if(len > 2)
      {
        printf("Too many args\n");
      }
      else if(argArr[1] != NULL)
      {
        char* env = malloc(sizeof(char*));
        env = getenv(argArr[1]);
        if(env != NULL)
          printf("%s\n",env);
        else
        {
          printf("env var not found\n");
        }
        free(env);
      }
      else
      {
        int i = 0;
        while(envp[i] != NULL)
        {
          printf("%s\n", envp[i]);
          i++;
        }
        //printf("%s\n",getenv(argArr[1]));
      }
    }
    else if (strcmp(built,"prompt") == 0)
    {

      if(argArr[1] == NULL)
      {
        printf("input prompt prefix: ");
        fgets(prefix2, sizeof(char*), stdin);
        prefix = prefix2;
        prefix[strlen(prefix)-1] = '\0';
      }
      else
      {
        prefix2 = argArr[1];
        prefix = prefix2;
        //prefix[strlen(prefix)-1] = '\0';
      }
    }
    else if (strcmp(built,"pwd") == 0)
    {
      printf("%s\n",cwd);
    }
    else if (strcmp(built,"setenv") == 0)
    {
      if(len > 3)
      {
        printf("Too many args\n");
      }
      else if(argArr[2] != NULL)
      {
        printf("you set the env var %s to the value %s\n",argArr[1],argArr[2]);
      }
      else if(argArr[1] != NULL)
      {
        printf("%s has been set as an env var\n",argArr[1]);
      }
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
        //printf("suze iz %i\n", len);
        if(argArr[i] != NULL)
        {
          //printf("dis length %lu\n", strlen(argArr[i]));
          //argArr[i][strlen(argArr[i]) - 1] = '\0';
          //printf("jes %s\n",argArr[i]);
          char* args = malloc(500);
          args = which(argArr[i],pathlist);
          printf("%s\n", args);
          free(args);
        }
      }
    }
    else if (strcmp(built,"watchuser") == 0)
    {
      node newNode = malloc(sizeof(node));
      newNode->username = argArr[1];
      if(head==NULL)
		  {
			printf("head is null\n");
			head = malloc(sizeof(node));
			head = newNode;
			head->next = NULL;
			head->prev = NULL;
			tail = newNode;
		}
		else
		{
			tail = addNode(tail, newNode);
			last->next = NULL;
		}
      if(watched == 0)
      {
        create_watchthread();
        watched = 1;
      }
    }
    histIndex++;

    }
  }
  return 0;
} /* sh() */

void catchCtrlC(int sig_num)
{
    signal(SIGINT, catchCtrlC);
}

//change all variable and function names
//get linked list code from first lab and create linked list of usernames
//my linked list instead of userlist
int watchuser_switch = 0;

node addNode(node last, node node1)
{
	node temp = malloc(sizeof(node));
	temp = last;
	temp->next =  malloc(sizeof(node));
	temp->next = node1;
	node1->prev = temp;
	//free(temp);
	return node1;
}

void create_watchthread() {
  pthread_t watchuser_pid;
  pthread_create(&watchuser_pid, NULL, watchuser_thread, "watchuser thread");
}

/*checks all of the users logged on*/
static void *watchuser_thread(void *param) {
  while(1) {
    userlist_check(head);
    sleep(20);
  }
}

void userlist_check(struct command_node *head) {
  struct command_node *curr = head;
  while(curr!=NULL)
  {
    checkuser_loggedin(curr->username);
    curr=curr->next;
  }
}

int checkuser_loggedin(char *username)
{
  struct utmpx *up;

  setutxent();			/* start at beginning */
  while (up = getutxent() )	/* get an entry */
  {
    if ( up->ut_type == USER_PROCESS )	/* only care about users */
    {
      if(strcmp(up->ut_user, username)==0) { /*check if user being watched matches*/
          printf("%s has logged on %s from %s \n", up->ut_user, up->ut_line, up ->ut_host);
      }
    }
  }
  return 0;
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
     strcat(cmd, command);
     if(access(cmd, F_OK) == 0)
     {
        //printf("%i\n",access(cmd, F_OK));
        commandVal = cmd;
        break;
      }
     pathlist = pathlist->next;
   }
   return commandVal;
}



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

void* externalCommand(char* name)
{
  system(name);
}
