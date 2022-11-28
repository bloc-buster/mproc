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

$blocbuster_path $infile $outfile $threshold $numInd $numSnps $headerRows $headerColumns $g1 $g2 $maxProcs $outFolder $semaphores

