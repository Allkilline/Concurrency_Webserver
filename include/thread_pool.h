#ifndef __THREAD_POOL_H_
#define __THREAD_POOL_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "request.h"
#include "io_helper.h"


// Forward declaration to store a pointer in thread_pool
struct _scheduler;
typedef struct _scheduler scheduler;

// Thread pool struct
//
// For Part 1: Store number of worker threads and the pthread_t array.
// For Part 2: also store a pointer to the scheduler so workers can
// fetch work from it.

typedef struct __thread_pool {
	int num_threads;
	pthread_t *threads;

	struct _scheduler *sch;		// set by wserver.c before starting threads

} thread_pool;

thread_pool* init_thread_pool(int n);
void* thread_function(void *_arg);
void start_thread_work(thread_pool* pool, scheduler* sch);

#endif // __THREAD_POOL_H_
