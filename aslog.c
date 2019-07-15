#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct logframe_ {
	void *data;
	bool final_frame;
} aslog_Logframe;

typedef struct logger_ {
	aslog_Logframe *buffer;
	bool (*callback_log)(void *);
	size_t buffer_size;

	pthread_t thread_id; /**< Thread ID of the log worker thread */
	sem_t buffer_full; /**< Semaphore to signal when the log buffer is full */
	sem_t buffer_empty; /**< Semaphore to signal when the log buffer is empty */
	int buffer_write_index; /**< Where to write into the log buffer */
	int buffer_read_index; /**< Where to read from the log buffer */
	int thread_return_status; /**< Signals successfull termination of the log worker thread */
} aslog_Logger;

typedef aslog_Logger * aslog_Logger_t;

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

aslog_Logger_t aslog_init(size_t _buffer_size, bool (*logger)(void *)){
	aslog_Logger_t log = malloc(sizeof(aslog_Logger));
	log->callback_log = logger;
	log->buffer_size = _buffer_size;
	log->buffer = calloc(log->buffer_size, sizeof(aslog_Logframe));

	sem_init(&(log->buffer_full),0,log->buffer_size);
	sem_init(&(log->buffer_empty),0,0);
	log->buffer_write_index=0;
	log->buffer_read_index=0;
	if( pthread_create(&(log->thread_id), NULL, aslog_logging, (void *)log) == 0 ){
		return log;
	}else{
		free(log);
		return NULL;
	}
}

void *aslog_logging(void *arg){
	aslog_Logger_t log = (aslog_Logger_t)arg;
	int *ret_status=&(log->thread_return_status);
	*ret_status=-1;
	while(true){
		sem_wait(&(log->buffer_empty));
		if( !log->callback_log(log->buffer[log->buffer_read_index].data) ){
			printf("error writing to log\n");
			*ret_status=-1;
			return (void *)ret_status;
		}
		if( log->buffer[log->buffer_read_index].final_frame ){
			*ret_status=0;
			return (void *)ret_status;
		}
		log->buffer_read_index=(log->buffer_read_index+1)%log->buffer_size;
		sem_post(&(log->buffer_full));
	}
	return (void *)ret_status;
}

void aslog_log_enqueue(aslog_Logger_t log, void *data, bool final){
	sem_wait(&(log->buffer_full));
	log->buffer[log->buffer_write_index].data = data;
	log->buffer[log->buffer_write_index].final_frame = final;
	log->buffer_write_index=(log->buffer_write_index+1)%log->buffer_size;
	sem_post(&(log->buffer_empty));
}

bool aslog_shutdown(aslog_Logger_t log){
	int *ret;
	if( pthread_join(log->thread_id,(void **)&ret) != 0 || *ret != 0 ){
		printf("Err: logger shutdown failed\n");
		free(log);
		return false;
	}
	free(log);
	return true;
}
