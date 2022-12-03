#!/bin/bash

#SBATCH -p hpc3 
#SBATCH -n 1
#SBATCH --mem 32G
#SBATCH --output="/home/jjs3k2/pipeline/multiqueue/data/bfs-%j.out"

keephi_path=$1
infile=$2
nodes=$3
placeholder=$4
edges_to_keep=$5
outfile=$6
num_snps=$7

edges=`grep edge $infile | wc -l`
k=`echo "$edges * $edges_to_keep" | bc -l`
keep=${k%%.*}

srun $keephi_path $infile $nodes $edges $keep $outfile

