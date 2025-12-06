#include "thread_pool.h"
#include "scheduler.h"

// Initialize thread pool with workers number
//
thread_pool* init_thread_pool(int n) {
	if (n <= 0) {
		fprintf(stderr, "init_thread_pool: n must be > 0\n");
		exit(1);
	}

	thread_pool *pool = (thread_pool *) malloc(sizeof(thread_pool));
	if (!pool) {
		perror("malloc thread_pool");
		exit(1);
	}

	pool->num_threads = n;
	pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * n);

	if (!pool->threads) {
		perror("malloc threads");
		free(pool);
		exit(1);
	}

	pool->sch = NULL; 	// Will be set in start_thread_work
    	return pool;
}

// Thread function for workers
//
// For Parts 1 + 2: Workers repeatedly get a connetion fd from the scheduler, 
// handle the request, and close it.
//
void* thread_function(void *_arg) {
	thread_pool *pool = (thread_pool *)_arg;

	if (!pool || !pool->sch) {
		fprintf(stderr, "thread_function: invalid pool or missing scheduler\n");
		return NULL;
	}

	while (1) {
		int conn_fd = get_scheduler_work(pool->sch, pool);
		if (conn_fd < 0) {
			// could use negative signal shutdown
			continue;
		}

		request_handle(conn_fd);
		close_or_die(conn_fd);
	}

	return NULL;
}

// Function that is called by main thread
//
void start_thread_work(thread_pool* pool, struct _scheduler* sch) {

	if (!pool || !pool->threads || pool->num_threads <= 0) {
		fprintf(stderr, "start_thread_work: invalid pool\n");
		exit(1);
	}


	// Store scheduler pointer inside the pool so workers can use it later on
	pool->sch = (scheduler *)sch;	// Scheduler typedef from scheduler.
			
	for (int i = 0; i < pool->num_threads; i++) {
		int rc = pthread_create(&pool->threads[i], NULL, thread_function, pool);
		if (rc != 0) {
			fprintf(stderr, "pthread_create failed: %d\n", rc);
			exit(1);
		}

		// Not planned to join these threads.
		pthread_detach(pool->threads[i]);
	}
}
