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
        if (argc != 3) {
                printf("Err: Input format\n");
                printf("command numCPU group\n");
                return -1;
        }

        group = atoi(argv[2]);
        numCPU = atoi(argv[1]);
        if (group < 1 || group > 2) {
                printf("Err: group\n");
                printf("group is either 1 or 2\n");
                return -1;
        }
        syscall(378, numCPU, group);
	return 0;
}
