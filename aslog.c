#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct logframe_ {
	void *data;
	bool final_frame;
} aslog_Logframe;

aslog_Logframe *aslog_buffer;
bool (*aslog_callback_log)(void *);
size_t aslog_buffer_size;

pthread_t aslog_thread_id; /**< Thread ID of the log worker thread */
sem_t aslog_buffer_full; /**< Semaphore to signal when the log buffer is full */
sem_t aslog_buffer_empty; /**< Semaphore to signal when the log buffer is empty */
int aslog_buffer_write_index; /**< Where to write into the log buffer */
int aslog_buffer_read_index; /**< Where to read from the log buffer */
int aslog_thread_return_status; /**< Signals successfull termination of the log worker thread */

//private functions
void *aslog_logging(void *);

bool aslog_init(size_t _buffer_size, bool (*logger)(void *)){
	aslog_callback_log = logger;
	aslog_buffer_size = _buffer_size;
	aslog_buffer = calloc(aslog_buffer_size, sizeof(aslog_Logframe));

	sem_init(&aslog_buffer_full,0,aslog_buffer_size);
	sem_init(&aslog_buffer_empty,0,0);
	aslog_buffer_write_index=0;
	aslog_buffer_read_index=0;
	return pthread_create(&aslog_thread_id, NULL, aslog_logging, &aslog_thread_return_status) == 0;
}

void *aslog_logging(void *arg){
	int *ret_status=(int *)arg;
	*ret_status=-1;
	while(true){
		sem_wait(&aslog_buffer_empty);
		if( !aslog_callback_log(aslog_buffer[aslog_buffer_read_index].data) ){
			printf("error writing to hdf5 log\n");
			*ret_status=-1;
			return (void *)ret_status;
		}
		if( aslog_buffer[aslog_buffer_read_index].final_frame ){
			*ret_status=0;
			return (void *)ret_status;
		}
		aslog_buffer_read_index=(aslog_buffer_read_index+1)%aslog_buffer_size;
		sem_post(&aslog_buffer_full);
	}
	return (void *)ret_status;
}

void aslog_log_enqueue(void *data, bool final){
	sem_wait(&aslog_buffer_full);
	aslog_buffer[aslog_buffer_write_index].data = data;
	aslog_buffer[aslog_buffer_write_index].final_frame = final;
	aslog_buffer_write_index=(aslog_buffer_write_index+1)%aslog_buffer_size;
	sem_post(&aslog_buffer_empty);
}

bool aslog_shutdown(void){
	int *ret;
	if( pthread_join(aslog_thread_id,(void **)&ret) != 0 || *ret != 0 ){
		printf("Err: logger shutdown failed\n");
		return false;
	}
	return true;
}
