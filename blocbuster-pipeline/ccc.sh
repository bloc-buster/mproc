#!/bin/bash

#SBATCH -p hpc3 
#SBATCH -n 1
#SBATCH --mem 32G
#SBATCH --output="/home/jjs3k2/pipeline/queue/data/ccc-%j.out"
#SBATCH --time=1-00:00:00
#SBATCH --account=climerlab

infile=$1
outfile=$2
threshold=$3
numInd=$4
numSnps=$5
headerRows=$6
headerColumns=$7
srun /home/jjs3k2/BlocBuster/ccc/ccc $infile $outfile $threshold $numInd $numSnps $headerRows $headerColumns


