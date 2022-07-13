#!/bin/bash

#SBATCH -p Lewis 
#SBATCH -N 1
#SBATCH -c 24
#SBATCH --exclusive
#SBATCH --mem 0G
#SBATCH --time=0-03:00:00
#SBATCH --account=climerlab

#echo "ERROR! Overwrite ccc.sh with sbatch parameters for your local system, then comment out these lines."
#exit 1

if [[ "$#" -eq 1 ]]
then
	conclude=$1
	if [ "$conclude" == "true" ]
	then
		srun ./ccc "true"
	fi
	exit 0
elif [[ "$#" -eq 4 ]]
then
        outputfolder=$1
        outputfile=$2
        let numind=$3
        let numsnps=$4
        srun ./ccc $outputfolder $outputfile $numind $numsnps
        exit 0
elif [[ "$#" -ne 14 ]]
then
	echo "args $#"
	echo "usage: ./ccc.sh input.txt output.gml threshold numInd numSNPs numHeaderRows numHeaderCols granularity2 (default 7) max_simultaneous_processes (default 15) output_folder (default temp_output_files) snp1 snp2 step count"
	exit 1
fi
let numargs=$#
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
let snp1=$1
shift
let snp2=$1
shift
let step=$1
shift
let count=$1
shift

srun ./ccc $inputfile $outputfile $threshold $numind $numsnps $numheaderrows $numheadercols $granularity2 $maxprocesses $outputfolder $snp1 $snp2 $step $count
let status=$?
if [ "$status" != "0" ]
then
	echo "checksum error"
	exit 1
fi



