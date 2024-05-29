# Introduction

An important part of this course is learning how to benchmark code. Reliably
measuring programs is vital in optimizing their performance. We will provide
some general guidelines you must follow, and give an example setup for
benchmarking code. If you prefer using different technology, feel free to do
that.

# General guidelines

Try to repeat each experiment at least 10 times, and make sure each experiment
runs for at least one second. Show both the average, and some measure of
variability (standard devation or minimum and maximum) in your report.

# Specific setup

I usually like to send only the compute rate to stdout, and send all other
information such as verifying the results to stderr. That way the relevant
numbers can be easily seperated and processed. You will see that the sequential
squaring program is setup that way.

## Install variance program

This is a small C program computing the mean and variance of numbers given on
stdin. Compile it

```
cd util
make
```

and copy the binary to a location in your path. For example

```
mkdir -p $HOME/.local/bin
cp bin/variance $HOME/.local/bin
echo "export PATH=$HOME/.local/bin:$PATH" >> $HOME/.bashrc
source $HOME/.bashrc
```

## Run experiments

Run your experiments for various problem sizes and (for OpenMP, MPI) amounts
of parallelism. Typically the latter is done for powers of two. The script
`bench_omp.sh` does this for OpenMP. You can run it with no arguments to
get a message on the usage. For example, you can do

```
./bench_omp.sh bin/square_omp 10 results square_1000000.omp 4 1000000
```

on your own machine, and

```
sbatch ./bench_omp.sh bin/square_omp 10 results square_1000000.omp 4 1000000
```

on the cluster. This produces a csv file `results/square_1000000.omp` which
you can then use for plotting. See `benchmarking/graph.tex` for some Latex code.
