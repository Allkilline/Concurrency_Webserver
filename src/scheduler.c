#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>		// recv
#include <errno.h>

#include "scheduler.h"
#include "request.h"
#include "io_helper.h"

#define STARVATION_SEC 3


// --- Helper: Compute file size for HTTP request --- //
//
// Must not consume data from the socket (request_handle() still needs is),
// so I use recv(..., MSG_PEED) to look at the request line without removing
// it from the kernel receive buffer.
//
static off_t compute_request_filesize(int fd) {

	char buf[MAXBUF];
	char method[MAXBUF], uri[MAXBUF], version[MAXBUF];
	char filename[MAXBUF], cgiargs[MAXBUF];
	ssize_t n;


	// Peek at the incoming request
	n = recv(fd, buf, MAXBUF - 1, MSG_PEEK);
	if (n <= 0) {
		// On error or closed connection, just treat it as size 0.
		return 0;
	}
	buf[n] = '\0';


	// Parse the request line: "GET /path HTTP/1.0"
	if (sscanf(buf, "%s %s %s", method, uri, version) != 3) {
		// Malformed; treat as size 0
		return 0;
	}


	// Use existing helper from request.c
	int is_static = request_parse_uri(uri, filename, cgiargs);

	struct stat sbuf;
	if (stat(filename, &sbuf) < 0) {
		// File not found or other error: size 0
		return 0;
	}


	if (is_static) {
		// For static content, SFF = based on file size
		return sbuf.st_size;

	} else {
		// For Dynamic content, I will set the request as size 0,
		// so they don't starve behind large static files.
		return 0;
	}
}


// --- Scheduler Implementation --- //


// Initialize main scheduler
// n = buffer capacity
scheduler* init_scheduler(int n) {

	if (n <= 0) {
		fprintf(stderr, "init_scheduler: n must be > 0\n");
		exit(1);
	}

	scheduler *sch = (scheduler *)malloc(sizeof(scheduler));
	if (!sch) { perror("malloc scheduler"); exit(1); }


	sch->buffer = (work *)malloc(sizeof(work) * n);
	if (!sch->buffer) { perror("malloc buffer"); exit(1); }


	sch->capacity	= n;
	sch->count 	= 0;

	pthread_mutex_init(&sch->mutex, NULL);
	pthread_cond_init(&sch->not_empty, NULL);
	pthread_cond_init(&sch->not_full, NULL);


	return sch;
}


// Insert a work for scheduler with fd
// When scheduler buffer is full, waits until worker takes one
// This is the PRODUCER size (main/acceptor thread).
void insert_scheduler_work(scheduler* sch, thread_pool* pool, int fd) {

	(void) pool;	// not used, but I kept it for API consistency


	pthread_mutex_lock(&sch->mutex);


	// Wait while buffer is full
	while(sch->count == sch->capacity){
		pthread_cond_wait(&sch->not_full, &sch->mutex);
	}

	
	// Build work item with SFF size
	work w;
	w.fd		= fd;
	w.filesize	= compute_request_filesize(fd);


	// Insert into buffer sorted by filesize (smallest first)
	int pos = 0;
	while (pos < sch->count &&
			sch->buffer[pos].filesize <= w.filesize) {
		pos++;
	}


	// Shift items to make room at pos
	for (int i = sch->count; i > pos; i--) {
		sch->buffer[i] = sch->buffer[i - 1];
	}
	sch->buffer[pos] = w;
	sch->count++;


	// Signal that buffer is not empty
	pthread_cond_signal(&sch->not_empty);

	pthread_mutex_unlock(&sch->mutex);
}

static int is_starved(work *w) {
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	time_t sec = now.tv_sec - w->arrival_time.tv_sec;
	return sec >= STARVATION_SEC;
}

// Get a work for scheduler with fd
// When scheduler buffer is empty, waits until work is filled
// This is the CONSUMER side (worker threads).
int get_scheduler_work(scheduler* sch, thread_pool* pool) {
	
	(void) pool;	// not used
			

	pthread_mutex_lock(&sch->mutex);


	// Wait while buffer is empty
	while(sch->count == 0) {
		pthread_cond_wait(&sch->not_empty, &sch->mutex);
	}

	// Choose what index to serve
	int idx = 0;		// pure SFF at index 0
	
	// Debug print: Before we shifting buffer and unlocking
	fprintf(stderr,
			"[scheduler] serving idx=%d fd=%d size=%ld (count=%d)\n\n",
			idx,
			sch->buffer[idx].fd,
			(long)sch->buffer[idx].filesize,
			sch->count);
	fflush(stderr);

	// First, look for any starved job
	int starved_idx = -1;
	for (int i = 0; i < sch->count; i++) {
		if (is_starved(&sch->buffer[i])) {
			if (starved_idx == -1) starved_idx = i;
		}
	}

	if (starved_idx != -1) {
		idx = starved_idx;		// Serve an old job, even if its large
	} else {
		idx = 0;			// purse SFF: smalled file at buffer
	}

	work w = sch->buffer[idx];
	
	// Shift remaining items left
	for (int i = 1; i < sch->count; i++) {
		sch->buffer[i - 1] = sch->buffer[i];
	}
	sch->count--;


	// Signal that there is space in buffer
	pthread_cond_signal(&sch->not_full);


	pthread_mutex_unlock(&sch->mutex);

	return w.fd;
}
