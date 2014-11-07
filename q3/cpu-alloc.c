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
	
	printf("input group\n");
	
	scanf("%d\n", group);
	
	printf("input num\n");
	
	scanf("%d\n", numCPU);
	
	syscall(378, numCPU, group);
	return 0;
}
