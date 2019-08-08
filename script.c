#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
void main(int argc, char *arg[])
{
   int pid;
   int ppid = fork();
   if(ppid<0)
   {
	printf("Fork failed");
	exit(1);
   }
   if(ppid>0)
   {
	wait(NULL);
        printf("iam parent %d\n",getpid());
        exit(0);
   }
else if(ppid==0)
{
	//wait(NULL);
	pid=fork();
        //printf("iam child\n");
        //printf("parent %d\n",getppid());

	if(pid==0)
	{
	printf("iam grandchild\n");

	}
	else
	{
	wait(NULL);

	printf("iam child\n");
	}
}
}
