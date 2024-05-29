# Default, for benchmarking BUILD=RELEASE make
BUILD ?= DEBUG

CC = gcc
MPICC = mpicc
RELEASE_FLAGS = -Ofast -march=native -mtune=native -Wall -Wextra -Werror
DEBUG_FLAGS = $(RELEASE_FLAGS) -ggdb -fsanitize=address -fsanitize=undefined
CFLAGS = $(${BUILD}_FLAGS)
LFLAGS = -lm

# Compute targets for all source files, so they do not have to be specified
# manually. Check out
# https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html
SEQ_SRC = $(wildcard src/seq/*.c)
SEQ = $(patsubst src/seq/%.c, bin/%_seq, $(SEQ_SRC))
OMP_SRC = $(wildcard src/omp/*.c)
OMP = $(patsubst src/omp/%.c, bin/%_omp, $(OMP_SRC))
MPI_SRC = $(wildcard src/mpi/*.c)
MPI = $(patsubst src/mpi/%.c, bin/%_mpi, $(MPI_SRC))
CL_SRC = $(wildcard src/ocl/*.c)
CL = $(patsubst src/ocl/%.c, bin/%_cl, $(CL_SRC))

# These are helper targets and files
.PHONY: all seq omp mpi cl clean

all: seq omp mpi cl

seq: $(SEQ)

omp: $(OMP)

mpi: $(MPI)

cl: $(CL)

bin/%_seq: src/seq/%.c
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS)

bin/%_omp: src/omp/%.c
	$(CC) $(CFLAGS) $^ -o $@ $(LFLAGS) -fopenmp

bin/%_mpi: src/mpi/%.c
	$(MPICC) $(RELEASE_FLAGS) $^ -o $@ $(LFLAGS) -fopenmp

# The OpenCL flag must be after the target
bin/%_cl: src/ocl/%.c bin/simple.o
	$(CC) -Iocl-simple $(CFLAGS) -D CL_TARGET_OPENCL_VERSION=220 $^ -o $@ -lOpenCL

bin/simple.o: ocl-simple/simple.c ocl-simple/simple.h
	$(CC) $(CFLAGS) -D CL_TARGET_OPENCL_VERSION=220 -c $< -o $@ -lOpenCL

clean:
	$(RM) bin/*
