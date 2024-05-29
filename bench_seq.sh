#!/bin/bash

#SBATCH --account=csmpistud
#SBATCH --partition=csmpi_short
#SBATCH --output=bench_seq.out

# If the script is used improperly, prints a short explanation
# and exits
if [ "$#" -lt 4 ]; then
    printf 'Usage: sbatch bench_seq.sh BINARY RUNS OUT_DIR OUT_NAME ARGS\n\n' >&2
    printf '\tBINARY: sequential program to benchmark\n' >&2
    printf '\t        Assumes only GFLOPS/s is printed to stdout\n\n' >&2
    printf '\tRUNS: How often to run the benchmark\n\n' >&2
    printf '\tOUT_DIR: Directory to store benchmark results\n\n' >&2
    printf '\tOUT_NAME: Name of benchmark csv\n\n' >&2
    printf '\tARGS: Commandline arguments for the binary\n\n' >&2
    exit 1
fi

# Parse commandline arguments (TODO get rid of bash-specific shift)
binary="$1"                         # Program to benchmark
shift
runs="$1"                           # How often to repeat each experiment
shift
outdir="$1"
shift
outname="$1"
shift

args=$(printf '%s ' "$@")           # Arguments for binary
outfile="${outdir}/${outname}"      # File to write the results to

mkdir -p "$outdir"                  # Creates the directory for the outfile if
                                    # it does not yet exist

make clean                       # Make sure we do not have old binaries around
BUILD=RELEASE make seq -j        # Compile the OpenMP programs in RELEASE mode
                                 # (see Lesson1)

printf 'mean,stddev\n' > "${outfile}"  # Create header of csv file

{
    i=1          # This while loop repeats the experiment "$runs" times.
                 # The { } block around this groups the stdout of this loop.
                 # So for example if "$runs" = 4, and the Gflops/s are
                 # around 1.0, you could get something like
                 # "1.0 0.94 1.03 1.02"
    while [ $i -le "$runs" ]
    do
        # OMP_NUM_THREADS sets the number of cores to use
        OMP_NUM_THREADS="$p" "$binary" $args
        i=$(( i + 1 ))
    done
} | variance >> "${outfile}" # Take the grouped stdout and pipe it into
                             # a program called 'variance'
                             # (see Lesson2-benchmarking.md) on how to
                             # install.
                             # This will compute the mean and standard
                             # deviation, outputting it on stdout.
                             # The >> "${outfile}" then redirects stdout
                             # to outfile
