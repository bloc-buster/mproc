#!/bin/bash

#SBATCH -p hpc3 
#SBATCH -n 1
#SBATCH --mem 32G
#SBATCH --output="/home/jjs3k2/pipeline/multiqueue/data/bfs-%j.out"

bfs_path=$1
infile=$2
outfile=$3
srun $bfs_path $infile $outfile

