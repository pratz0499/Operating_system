#include<stdio.h>

int main()
{
int i,n,bt[100],wt[100],tat[100];
float  wt_sum=0,tat_sum=0;

	printf("Enter the number of process: ");
	scanf("%d",&n);

	printf("Enter the burst time: ");
	for(i=0;i<n;i++)
	{
	  scanf("%d",&bt[i]);
	}

wt[0]=0;
	for(i=0;i<n;i++)
	{
	  wt[i+1]=wt[i]+bt[i];
	  tat[i]=bt[i]+wt[i];
	  wt_sum=wt_sum+wt[i];
	  tat_sum=tat_sum+tat[i];
	}

	double avg_wt=printf("%lf\n",wt_sum/n);
	double avg_tat=printf("%lf\n",tat_sum/n);




return 0;
}
