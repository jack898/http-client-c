# Makefile for httpClient
# Author: Jack Burton
#
# Compiles simple HTTP GET client

# Executables to be built using "make all"
EXECUTABLES = httpClient

# Header files to include
INCLUDES = client.h

# Compiler
CC = gcc

# Compiler flags
CFLAGS = -g -std=gnu99 -Wall -Wextra -Werror -pedantic

# Linking flags
LDFLAGS = 

# Platform-specific libraries
ifeq ($(OS),Windows_NT)
	LIBS = -lws2_32
	RM = del
else
	LIBS =
	RM = rm -f
endif

# 'make all' will build all executables
all: $(EXECUTABLES)

# 'make clean' will remove all object and executable files
clean:
	$(RM) $(EXECUTABLES) *.o

# To get any .o, compile the corresponding .c
%.o: %.c $(INCLUDES)
	$(CC) $(CFLAGS) -c $< -o $@

# Individual executables
httpClient: client.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

.PHONY: all clean