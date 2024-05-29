# Introduction

This repository contains some examples and a Makefile for writing parallel
programs.
This course is taught in C because (along with Fortran) it is the dominant
language in HPC. If you are already proficient in C and \*nix environments,
you can skip this. If not, I hope this is of help.

# Installation

It is recommended to work on in a Unix-like environment (so Linux, 
(Free|Open|Net)BSD, Darwin, are fine, for Windows you will need WSL). 
You will need `make` and a C compiler. With `apt` (Debian package manager)

`sudo apt install build-essential`

should do the trick.

## OpenMP

Should work out of the box with all major compilers such as GCC, Clang and ICC.

## MPI

With `apt`

`sudo apt install libopenmpi-dev`

Easy to install from [source](https://github.com/open-mpi/ompi) as well.

## OpenCL

It can be tough to get OpenCL working on your laptop/PC, so using cn132 is
always an option. However, debugging is easier locally. First, figure out
if you have a discrete GPU. If not, you can (probably) run OpenCL on the
integrated graphics of your CPU. 

You will need three things: drivers, an ICD loader, and development
headers. 

With `apt` you can get free variants with

`sudo apt install mesa-opencl-icd opencl-c-headers`

Usually the vendor of your GPU also has a non-free ICD implementation and 
drivers.

You can check with 'clinfo' whether the device is recognized.

# Building programs

Put your sequential programs in `src/seq`, your OpenMP programs in `src/omp`,
your MPI programs in `src/mpi` and your OpenCL programs in `src/ocl`. You can
build all programs with `make` or e.g. the OpenMP programs with `make omp`.
Have a look at the Makefile.

By default all program will build with runtime error checking. Use this for
development, but when benchmarking, you should build with environment variable
BUILD set to RELEASE. E.g.

```
make clean
BUILD=RELEASE make -j
```

OpenMPI has some memory leaks, you do not need to worry about that.
