
#include "get_path.h"
#include "ll.h"


int pid;
int sh( int argc, char **argv, char **envp);
char *which(char *command, struct pathelement *pathlist);
void *where(char *command, struct pathelement *pathlist);
void list ( char *dir );
void printenv(char **envp);
void cd(char**arr);
void catchCtrlC(int sig_num);
void* externalCommand(char* name);
void* watchUsers();

void create_watchthread();
static void *watchuser_thread(void *param);
void userlist_check(node head);
int checkuser_loggedin(char *username);

#define PROMPTMAX 32
#define MAXARGS 10
