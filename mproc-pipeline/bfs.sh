#!/bin/bash

bfs_path=$1
infile=$2
outfile=$3
srun $bfs_path $infile $outfile

