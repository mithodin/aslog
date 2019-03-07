#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct logframe_ {
	void *data;
	bool final_frame;
} Logframe;

Logframe *buffer;
bool (*callback_log)(void *);
size_t buffer_size;

pthread_t log_thread_id; /**< Thread ID of the log worker thread */
sem_t log_buffer_full; /**< Semaphore to signal when the log buffer is full */
sem_t log_buffer_empty; /**< Semaphore to signal when the log buffer is empty */
int log_buffer_write_index; /**< Where to write into the log buffer */
int log_buffer_read_index; /**< Where to read from the log buffer */
int thread_return_status; /**< Signals successfull termination of the log worker thread */

//private functions
void *log_logging(void *);

bool aslog_init(size_t _buffer_size, bool (*logger)(void *)){
	callback_log = logger;
	buffer_size = _buffer_size;
	buffer = calloc(buffer_size, sizeof(Logframe));

	sem_init(&log_buffer_full,0,buffer_size);
	sem_init(&log_buffer_empty,0,0);
	log_buffer_write_index=0;
	log_buffer_read_index=0;
	return pthread_create(&log_thread_id, NULL, log_logging, &thread_return_status) == 0;
}

void *log_logging(void *arg){
	int *ret_status=(int *)arg;
	*ret_status=-1;
	while(true){
		sem_wait(&log_buffer_empty);
		if( !callback_log(buffer[log_buffer_read_index].data) ){
			printf("error writing to hdf5 log\n");
			*ret_status=-1;
			return (void *)ret_status;
		}
		if( buffer[log_buffer_read_index].final_frame ){
			*ret_status=0;
			return (void *)ret_status;
		}
		log_buffer_read_index=(log_buffer_read_index+1)%buffer_size;
		sem_post(&log_buffer_full);
	}
	return (void *)ret_status;
}

void aslog_log_enqueue(void *data, bool final){
	sem_wait(&log_buffer_full);
	buffer[log_buffer_write_index].data = data;
	buffer[log_buffer_write_index].final_frame = final;
	log_buffer_write_index=(log_buffer_write_index+1)%buffer_size;
	sem_post(&log_buffer_empty);
}

bool aslog_shutdown(void){
	int *ret;
	if( pthread_join(log_thread_id,(void **)&ret) != 0 || *ret != 0 ){
		printf("Err: logger shutdown failed\n");
		return false;
	}
	return true;
}
