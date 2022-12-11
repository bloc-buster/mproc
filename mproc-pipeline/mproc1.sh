#!/bin/bash

blocbuster_path=$1
shift
infile=$1
shift
outfile=$1
shift
threshold=$1
shift
numInd=$1
shift
numSnps=$1
shift
headerRows=$1
shift
headerColumns=$1
shift
g1=$1
shift
g2=$1
shift
maxProcs=$1
shift
outFolder=$1
shift
semaphores=$1
shift
slurm_partition=$1
shift
slurm_mem=$1
shift
slurm_time=$1
shift

params="$blocbuster_path -P $slurm_partition -M $slurm_mem -T $slurm_time $infile $outfile $threshold $numInd $numSnps $headerRows $headerColumns $g1 $g2 $maxProcs $outFolder $semaphores"

pidlist=`srun $params`
echo $pidlist

