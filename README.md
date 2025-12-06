![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)


# Concurrency Web Server

A multi-threaded HTTP/1.0 web server written in C, featuring a configurable thread pool, bounded scheduler queue, and secure filesystem sandboxing.
Originally built for a university concurrency/OS course, later reorganized and refined for clarity and maintainability.

---

## Overview
This project implements a miniature but realistic HTTP web server supporting concurrent clients via:

* **Thread pool** (fixed number of worker threads)
* **Scheduler / bounded buffer** (producer / consumer queue)
* **Static file serving** from a configurable basedir
* **SFF scheduling** Follows a First Job First (SJF) scheduling
* **Parallel request handling** using POSIX threads.

Also includes:  
* A small HTTP client (`wclient`)
* Test scripts for concurrency
* Example static files (`small.html`, `huge.html`, etc)

The goal is to demonstrate correct synchronization, scheduling and server concurrency design using C and POSIX threads.

---

## Project Structure

```text

Concurrency_Webserver/
├── src/                  # C source files
│   ├── wserver.c         # Main server logic, argument parsing, setup
│   ├── wclient.c         # Simple HTTP client for testing
│   ├── io_helper.c       # Socket utilities, HTTP helpers
│   ├── request.c         # request_handle(), parse URI, serve files
│   ├── scheduler.c       # Bounded buffer queue (producer/consumer)
│   └── thread_pool.c     # Worker threads that process requests
│
├── include/              # C header files
│   ├── io_helper.h
│   ├── request.h
│   ├── thread_pool.h
│   └── scheduler.h
│
├── www/                  # Static HTML content served to clients
│   ├── small.html
│   ├── huge.html
│   └── big.html
│
├── tests/                # Automated test scripts
│   └── test_sff.sh       # Stress test for SFF scheduling
│
├── docs/                 # Project report / PDF writeup
│   └── Webserver_Report.pdf  (example)
│
├── Makefile              # Build file
└── README.md             # Documentation (this file)
```

---

## Core Components

`wserver.c` - **Main Server**
* Parses command-line flags (`-d`, `-p`, `-t`, `-b`)
* Sets basedir (`chdir()`)
* Opens the listening socket
* Spawns the thread pool
* Pushes incoming connections into the scheduler queue

`scheduler.c` - **Bounded request queue**
* Implements the producer/consumer model
* Controls how requests are ordered (SFF)
* Blocks when the buffer is full/empty

`thread_pool.c` - **Worker threads**
* Dequeues file descriptors from the scheduler
* Calls `request_handle(fd)`
* Runs in parallel to handle multiple clients

`request.c` - **HTTP request handler**
* Parses the URI
* Validates paths for security
* Serves static files
* Sends HTTP headers + body

`io_helper.c` - **Networking utilities**
* Safe wrappers for socket creationg, `accept()`, reading/writing
* Helper functions like `open_listen_fd_or_die()`

`wclient.c` - **Test client**
* Sends HTTP/1.0 requests to the server
* Used by `tests/` for concurrency load testing.


---


## Build and Run

### **Build**

From the project root

```bash
make clean
make
```

This produces:
* `wserver`
* `wclient`
* `spin.cgi` (not tested yet)


### **Run the Server**

```bash
./wserver -d www -p 8000 -t 4 -b 5
```

- Configurable:
  - **basedir** (`-d`): directory to serve files from (e.g. `www/`)
  - **port** (`-p`): listening port
  - **threads** (`-t`): number of worker threads
  - **buffers** (`-b`): size of the scheduler queue


### **Test with Curl**
In another terminal:

```bash
curl http://localhost:8000/small.html
curl http://localhost:8000;huge.html
```

### **Run Automated Tests
With server running:

```bash
cd tests

./test_sff.sh        # Tests SFF (Smallest File First) scheduling under mixed workloads.
                     # Sends many concurrent small + huge file requests to verify that
                     # small files are served quickly even when large files are present.

./test_starvation.sh # Tests for starvation behavior.
                     # Ensures that large-file requests eventually get served even when
                     # a continuous stream of small-file requests is present.

./test_traversal.sh  # Tests scheduler queue traversal and ordering.
                     # Verifies correct dequeue order (FIFO/SFF), checks for queue
                     # pointer bugs, and ensures the bounded buffer behaves correctly.
```
This will launch multiple concurrent `wclient` calls to evaluate the scheduling behavior.

---

## Author

**Felipe Campoverde**
Computer Science @ Virginia Tech

* Portfolio: https://fcampoverdeg.dev
* Email: fcampoverdeg@gmail.com

---

## License

This project is released under the **MIT License**.

---
