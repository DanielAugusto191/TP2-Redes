#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const int bufz = 2048;

int main(void){
	printf("%02d\n", 15);

	char t[50];
	sprintf(t, "%02d ", 1);
	sprintf(t, "%02d ", 2);
	printf("%s\n",t);
	int a = atoi("01");
	printf("%d\n", a);
}

