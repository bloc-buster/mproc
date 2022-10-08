#!/bin/bash

let numargs=$#
# if second run
if [[ "$#" -eq 1 ]]
then
	# get root path to next file
	runpath=$1
	command "$runpath/mproc" "true"
	exit 0
# otherwise validate number of args
elif [[ "$#" -ne 17 ]]
then
	echo "args $#"
	echo "usage: ./mproc.sh input.txt output.gml threshold numInd numSNPs numHeaderRows numHeaderCols granularity2 (default 7) max_simultaneous_processes (default 15) output_folder (default temp_output_files) snp1 snp2 step count xedge yedge"
	exit 1
fi
# read command line args (from main.sh)
inputfile=$1
shift
outputfile=$1
shift
threshold=$1
shift
let numind=$1
shift
let numsnps=$1
shift
let numheaderrows=$1
shift
let numheadercols=$1
shift
let granularity2=$1
shift
let maxprocesses=$1
shift
outputfolder=$1
shift
# partial gml file count
let count=$1
shift
let step=$1
shift
let xstart=$1
shift
let xstop=$1
shift
let ystart=$1
shift
let ystop=$1
shift
# root path to next file
runpath=$1
shift
# if multiprocessing
if [[ $granularity2 -gt 0 ]]
then
   args="$runpath/mproc $inputfile $outputfile $threshold $numind $numsnps $numheaderrows $numheadercols $granularity2 $maxprocesses $outputfolder $count $step $xstart $xstop $ystart $ystop $runpath"
   command $args
# otherwise in wrong file
else
   echo "error - mproc.sh running ccc file"
   exit 1
fi
# check status of mproc.cpp
let status=$?
if [ "$status" != "0" ]
then
	echo "checksum error"
	exit 1
fi

