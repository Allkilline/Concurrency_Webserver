#ifndef __SCHEDULER_H_
#define __SCHEDULER_H_

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h> // needed for mutex/cond
		     
#include "thread_pool.h"
		     

// Work struct for threads
//
// Struct should contain file descriptor of file
// and size of file for sorting
//
typedef struct _work {
	int fd;				// connection file descriptor
	off_t filesize;			// not used yet; placeholder for SFF
	struct timespec arrival_time;	// when enters the scheduler
    
} work;

// Main scheduler struct
//
typedef struct _scheduler {
	work *buffer;			// Circular buffer for works
	int capacity;			// man number of items
	int count;			// current number of items

	pthread_mutex_t mutex;		// protects all fields above
	pthread_cond_t not_empty;	// signaled when buffer transitions 0 -> 1
	pthread_cond_t not_full;	// signaled when buffer transitions full -> not full
    
} scheduler;


scheduler* init_scheduler(int n);
void insert_scheduler_work(scheduler* sch, thread_pool* pool, int fd);
int get_scheduler_work(scheduler* sch, thread_pool* pool);

#endif //__SCHEDULER_H_
