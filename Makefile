# Simple Makefile for the concurrency web server project

CC      = gcc
SRC_DIR = src
INC_DIR = include

CFLAGS  = -g -Wall -Wextra -I$(INC_DIR)

OBJS    = wserver.o wclient.o request.o io_helper.o thread_pool.o scheduler.o

.PHONY: all clean

# Build everything
all: wserver wclient spin.cgi

# Server binary
wserver: wserver.o request.o io_helper.o thread_pool.o scheduler.o
	$(CC) $(CFLAGS) -o $@ $^

# Client binary
wclient: wclient.o io_helper.o
	$(CC) $(CFLAGS) -o $@ $^

# CGI program
spin.cgi: $(SRC_DIR)/spin.c
	$(CC) $(CFLAGS) -o $@ $<

# Generic rule: build .o from src/*.c
%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build artifacts
clean:
	-rm -f $(OBJS) wserver wclient spin.cgi

