#!/bin/bash

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

$keephi_path $infile $nodes $edges $keep $outfile

