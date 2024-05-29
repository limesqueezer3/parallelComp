#!/bin/bash

#SBATCH --account=csmpistud
#SBATCH --partition=csmpi_fpga_short
#SBATCH --cpus-per-task=32
#SBATCH --output=bench_mpi.out

# If the script is used improperly, prints a short explanation
# and exits
if [ "$#" -ne 3 ]; then
    printf 'Usage: collect_mpi.sh OUT_DIR OUT_NAME N\n\n' >&2
    printf '       Companion script to bench_mpi.sh. Run only\n' >&2
    printf '       after that script has given you all results.\n\n' >&2
    printf '\tOUT_DIR: Directory to store benchmark results\n\n' >&2
    printf '\tOUT_NAME: Name of benchmark csv\n\n' >&2
    printf '\tN: maximum number of nodes\n\n' >&2
    exit 1
fi

outdir="$1"
outname="$2"
nmax="$3"

outfile="${outdir}/${outname}.csv"

printf 'ranks,gflops,stddev' > "$outfile"

nodes=1
ntasks=8
while [ $nodes -le "$nmax" ]
do
    printf '\n%s,' "$ntasks" >> "$outfile"
    tr -d '\n' < "${outdir}/${outname}_${ntasks}" >> "$outfile"
    nodes=$(( nodes * 2 ))
    ntasks=$(( ntasks * 2 ))
done
