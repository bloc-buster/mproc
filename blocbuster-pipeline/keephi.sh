#!/bin/bash

#SBATCH -p hpc3 
#SBATCH -n 1
#SBATCH --mem 32G
#SBATCH --output="./data/keephi-%j.out"

infile=$1
nodes=$2
placeholder=$3
edges_to_keep=$4
outfile=$5

edges=`grep edge $infile | wc -l`
k=`echo "$edges * $edges_to_keep" | bc -l`
keep=${k%%.*}

srun ../blocbuster/keepHi/keepHi $infile $nodes $edges $keep $outfile

