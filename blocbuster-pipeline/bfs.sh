#!/bin/bash

#SBATCH -p hpc3 
#SBATCH -n 1
#SBATCH --mem 32G
#SBATCH --output="/home/jjs3k2/pipeline/queue/data/bfs-%j.out"

infile=$1
outfile=$2
srun /home/jjs3k2/BlocBuster/bfs/bfs $infile $outfile

