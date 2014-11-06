#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sched.h>

int main(int argc, char **argv) 
{

	struct sched_param param;
	param.sched_priority = 99;
	int i = 0;
	scanf("%i", &i);
	sched_setscheduler(i, 6, & param);
	printf("OK");
	return 0;
}
