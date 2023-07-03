#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const int bufz = 2048;

int main(void){
	printf("%02d\n", 15);

	char t[50];
	sprintf(t, "%02d ", 1);
	sprintf(t, "%02d ", 2);
	printf("%s\n",t);
	int a = atoi("01");
	printf("%d\n", a);

	time_t now = time(NULL);
	struct tm *tm_struct = localtime(&now);
	int hour = tm_struct->tm_hour;
	int mm = tm_struct->tm_min;
	char tteste[bufz];
	char b[] = "limao";
	sprintf(tteste, "%d:%d -> %s", hour, mm, b);
	printf("%s\n", tteste);

}

