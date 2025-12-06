#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h>

#include "request.h"
#include "io_helper.h"

#include "thread_pool.h"
#include "scheduler.h"

char default_root[] = ".";

// Global so thread_pool.c can use it via 'extern'
int listen_fd = -1;

// signal handler to close the listening socket gracefully on shutdown
void cleanup(int signal) {
    (void) signal;
    if (listen_fd != -1) {
        close(listen_fd);
        printf("socket closed gracefully\n");
    }
    exit(0);
}

// ./wserver [-d <basedir>] [-p <portnum>] [-t threads] [-b buffers]
int main(int argc, char *argv[]) {

    char *root_dir = default_root;
    int port = 10000;		// default port
    int num_threads = 1;	// default # worker threads
    int buffer_size = 1;	// default scheduler buffer size

    int c;
    while ((c = getopt(argc, argv, "d:p:t:b:")) != -1) {
		switch (c) {
			case 'd':
				root_dir = optarg;
				break;
			case 'p':
				port = atoi(optarg);
				break;
			case 't':
				num_threads = atoi(optarg);
				break;
			case 'b':
				buffer_size = atoi(optarg);
				break;
			default:
				fprintf(stderr, "usage: wserver [-d basedir] [-p port] [-t threads] [-b buffers]\n", argv[0]);
				exit(1);
		}
	}

    // Check correctness of parameters
    if (num_threads <= 0 || buffer_size <= 0) {
	    fprintf(stderr, "threads and buffers must be positive integers\n");
	    exit(1);
    }


    // run out of this directory
    chdir_or_die(root_dir);




    // log current working directory
    char cwd[MAXBUF];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("current working directory: %s\n", cwd);
    } else {
        fprintf(stderr, "getcwd() error\n");
        exit(1);
    }


    // register signal handlers
    signal(SIGINT, cleanup);  // ctrl + c
    signal(SIGTSTP, cleanup); // ctrl + z
    signal(SIGTERM, cleanup); // kill

    
    // Open listening socker
    listen_fd = open_listen_fd_or_die(port);

    thread_pool *pool = init_thread_pool(num_threads);
    scheduler   *sch  = init_scheduler(buffer_size);

    // Start worker threads first
    start_thread_work(pool, sch);

    // Main thread is now ONLY the acceptor (producer)
    while (1) {
		struct sockaddr_in client_addr;
		int client_len = sizeof(client_addr);
		int conn_fd = accept_or_die(listen_fd, (sockaddr_t *) &client_addr, (socklen_t *) &client_len);
        // request_handle(conn_fd);	( I rather just do "fd to scheduler" in this file)
	insert_scheduler_work(sch, pool, conn_fd);
	/* Workers will call request_handle(conn_fd) and close (conn_fd) */
    }

    
    return 0;
}
