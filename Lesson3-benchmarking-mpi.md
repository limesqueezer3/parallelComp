# Introduction

This explains how to benchmark MPI code with some scripts we have prepared for 
you, similar to the OpenMP scripts.
That section can be skipped if you want to do it your own way, but do not skip
the next section!

# Timing MPI code

MPI has its own timer, `double MPI_Wtime()` which should be used instead
of the timer used in the sequential code. We are interested in the total
execution time, but we can only measure time at a specific node. For this
reason, it is important that all ranks arrive at `MPI_Wtime()` at the same time.
This can be achieved by calling `MPI_Barrier(MPI_COMM_WORLD)` immediately before
`MPI_Wtime()`.

# Benchmark scripts

You will need different `#SBATCH` directives depending on the number of nodes
you run your program on. That means it is not possible to benchmark on
1, 2, 4, 8 nodes with just one batch script. The script `bench_mpi.sh` will
automatically create multiple batch scripts from `bench_mpi.sh.template` and
submit these to SLURM. 

Run `./bench_mpi.sh` with no arguments to get an explanation about the options
`BINARY RUNS OUT_DIR OUT_NAME N ARGS`.

For example, if you want to benchmark the program
`bin/square_mpi` with input `1000000000`, for 10 runs, on 1, 2, 4, 8 nodes,
you can execute 

```
./bench_mpi.sh bin/square_mpi 10 results square 8 1000000000
```

This will then create `square_<n>.sh` for n = 8, 16, 32, 64 and submit those
to SLURM. The configuration is 8 ranks per node, which is why n is 8, 16, 32, 64
instead of 1, 2, 4, 8.

After these scripts have finished (check `squeue --me`), and completed 
succesfully (no errors in `<OUTNAME>_<n>.out`, `<OUTDIR>/<OUTNAME>_<n>` looks
reasonable), you can accumulate the results into a csv by running

`./collect_mpi.sh` with matching arguments. So for this example,

```
./collect_mpi.sh results square 8
```

which produces a csv `results/square.csv`.
