#!/bin/bash

#SBATCH -p hpc3 
#SBATCH -n 1
#SBATCH --mem 32G
#SBATCH --output="./data/bfs-%j.out"

infile=$1
outfile=$2
srun ../blocbuster/bfs/bfs $infile $outfile

