#include <stdlib.h>
#include <stdio.h>
#include "aslog.h"

#define BUFSIZE 5

bool logger(void *arg){
	int lognumber = *(int *)arg;
	printf("Log No. %d\n",lognumber);
	free(arg);
	return true;
}

int main(void){
	aslog_Logger_t log = aslog_init(BUFSIZE, logger);
	if( !log ){
		printf("Failed to start logging.\n");
		return 1;
	}
	int *k;
	for(int i=0;i<10;++i){
		k = malloc(sizeof(int));
		*k = i;
		printf("queing k = %d\n",*k);
		aslog_log_enqueue(log, (void *)k, false);
	}
	k = malloc(sizeof(int));
	*k = 10;
	aslog_log_enqueue(log, (void *)k, true);

	aslog_shutdown(log);
	return 0;
}
