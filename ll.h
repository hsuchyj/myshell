#ifndef ll
#define ll

struct linkl
{
	char* username;
  int watch;
	struct linkl* next;
	struct linkl* prev;
};

typedef struct linkl* node;

node addNode(node last, node node1);
void turnOffWatch(char* username, node first);

#endif
