#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sched.h>
#include "../flo-kernel/include/linux/sched.h"

int main(int argc, char **argv) 
{

	struct sched_param p;
	//p.sched_priority = 99;
	int i = 0;
	scanf("%d", &i);
	printf("%d", i);
	pid_t t = (pid_t) i;
	printf("%d",sched_setscheduler(t, 6, &p));
		printf("OK");
	printf(strerror(errno));
	return 0;
}
