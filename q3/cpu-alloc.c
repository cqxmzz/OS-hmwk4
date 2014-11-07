#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>


int main(int argc, const char *argv[])
{
	int group;
	int numCPU;
	int i;
	printf("input group\n");
	scanf("%d", &group);
	printf("input num\n");
	scanf("%d", &numCPU);
	i = syscall(378, numCPU, group);
	printf("%d", i);
	return 0;
}
