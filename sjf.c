#include<stdio.h>
void swap(int* a,int* b)
{
	int t=*a;
	*a=*b;
	*b=t;
}

int partition(int bt[],int l,int h)
{
	int pivot=bt[h];
	int i=l-1;
	for(int j=l;j<=h-1;j++)
	{
		 if(bt[j]<pivot)
	 	 {
	 	   i++;
	 	   swap(&bt[i],&bt[j]);
	 	 }
	}
	swap(&bt[i+1],&bt[h]);
	return (i+1);
}
void quicksort(int bt[],int l,int h)
{
	if(l<h)
	{
	 int pi=partition(bt,l,h);

	 quicksort(bt,l,pi-1);
	 quicksort(bt,pi+1,h);
	}
}

int main()
{ 
	float wt_sum,tat_sum;
	int i,n,bt[10],wt[10],tat[10];

	printf("Enter the total number of process: ");
	scanf("%d",&n);

	printf("Enter the burst time: ");
	for(i=0;i<n;i++)
	{
	 scanf("%d",&bt[i]);
	}

	quicksort(bt,0,n-1);
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

	
}
	
