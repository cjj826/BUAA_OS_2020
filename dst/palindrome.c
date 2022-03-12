#include<stdio.h>
int a[10];
int main()
{
    int n;
    scanf("%d",&n);
    int tot = 0;
	while(n/10) {
		a[tot++] = n - n/10*10;
		n /= 10; 
	} 
	a[tot] = n;
	int i = 0;
	int j = tot;
	while(i < j) {
		if(a[i] != a[j])
		break;
		i++;
		j--;
	}
    if(i >= j){
        printf("Y");
    }else{
        printf("N");
    }
    return 0;
}
