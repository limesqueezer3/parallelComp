#!/bin/bash

# If the script is used improperly, prints a short explanation
# and exits
if [ "$#" -lt 5 ]; then
    printf 'Usage: bench_mpi.sh BINARY RUNS OUT_DIR OUT_NAME N ARGS\n\n' >&2
    printf '\tBINARY: MPI program to benchmark\n' >&2
    printf '\t        Assumes only GFLOPS/s is printed to stdout\n\n' >&2
    printf '\tRUNS: How often to run the benchmark\n\n' >&2
    printf '\tOUT_DIR: Directory to store benchmark results\n\n' >&2
    printf '\tOUT_NAME: Name of benchmark csv\n\n' >&2
    printf '\tN: Maximum number of nodes. Uses 8 ranks per node\n\n' >&2
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
nmax="$1"                           # Will experiment on 1, 2, ..., nmax
                                    # nodes, with 8 ranks per node
shift
args=$(printf '%s ' "$@")           # Arguments for binary

mkdir -p "$outdir"                  # Creates the directory for the outfile if
                                    # it does not yet exist

make clean

nodes=1
ntasks=8
while [ $nodes -le "$nmax" ] # This loop iterates over 1, 2, 4, ..., nmax nodes
do
    # Create and submit batch script from template
    sed "s/NODES/$nodes/g" bench_mpi.sh.template > "${outname}_${ntasks}".sh
    sed -i "s/NTASKS/$ntasks/g"    "${outname}_${ntasks}".sh
    sed -i "s/RUNS/$runs/g"        "${outname}_${ntasks}".sh
    sed -i "s/RUNS/$runs/g"        "${outname}_${ntasks}".sh
    sed -i "s/OUTDIR/$outdir/g"    "${outname}_${ntasks}".sh
    sed -i "s/OUTNAME/$outname/g"  "${outname}_${ntasks}".sh
    sbatch "${outname}_${ntasks}".sh "$binary" $args

    nodes=$(( 2 * nodes ))
    ntasks=$(( 2 * ntasks ))
done
