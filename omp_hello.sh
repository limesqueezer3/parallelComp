#!/bin/sh

#SBATCH --account=csmpistud
#SBATCH --cpus-per-task=32
#SBATCH --partition=csmpi_fpga_short
#SBATCH --time=00:05:00
#SBATCH --output=hello_omp.out

# Compile on the machine, not the head node
BUILD=RELEASE make bin/hello_world_omp

outfile="results/hello_omp.txt"

true > $outfile

P=1
while [ $P -le 32 ]
do
    {
        printf '%d threads:\n' "$P"
        OMP_NUM_THREADS="$P" bin/hello_world_omp
    } >> $outfile
    P=$(( 2 * P ))
done
