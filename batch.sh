#!/bin/bash

if [[ "$#" -eq 1 ]]
then
	slurmoutfile=$1
	command sbatch --output="$slurmoutfile-%j.out" ./ccc.sh "true" 
	exit 0
elif [[ "$#" -eq 4 ]]
then
        outputfolder=$1
        outputfile=$2
        let numind=$3
        let numsnps=$4
        command sbatch --output="$outputfile-%j.out" ./ccc.sh $outputfolder $outputfile $numind $numsnps
        exit 0
elif [[ "$#" -lt 7 || "$#" -gt 12 ]]
then
	echo "$#"
	echo "usage: ./batch.sh input.txt output.gml threshold numInd numSNPs numHeaderRows numHeaderCols granularity1 (default 1) granularity2 (default 7) max_simultaneous_processes (default 15) temp_output_folder (default temp_output_files)"
	echo "alternate (only for conclude): ./batch.sh true"
	exit 1
fi

let numargs=$#
inputfile=$1
shift
outputfile=$1
shift
slurmoutfile="$outputfile-%j.out"
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
let granularity1=1
if [ $numargs -ge 8 ]
then
	let granularity1=$1
	if [[ $granularity1 -gt 7 ]]
	then
		echo "granularity too high, no higher than 7"
		exit 0
	fi
	shift
fi
let granularity2=7
if [ $numargs -ge 9 ]
then
	let granularity2=$1
	if [[ $granularity1 -gt 1 && $granularity2 -gt 7 ]]
	then
		echo "granularity too high, no higher than 7"
		exit 0
	fi
	shift
fi
let maxprocesses=15
if [ $numargs -ge 10 ]
then
	let maxprocesses=$1
	shift
fi
outputfolder="temp_output_files"
if [ $numargs -ge 11 ]
then
	outputfolder=$1
	shift
fi

params="#define num_ind $numind\n#define num_snps1 $numsnps\n#define num_snps2 $numsnps\n#define gml_file \"$outputfile\"\n#define temp_folder \"$outputfolder\""
command echo -e $params > params.h
sleep 1
command srun make clean
sleep 1
command srun make mproc
sleep 1
command srun make helper
sleep 1
let status=$?
if [ "$status" != "0" ]
then
	echo "compilation error - exiting"
	exit 1
fi
command rm -rf $outputfolder
sleep 1
command mkdir $outputfolder
let status=$?
if [ "$status" != "0" ]
then
	echo "error - could not make output folder"
	exit 1
fi
sleep 1
command touch $outputfile
let status=$?
if [ "$status" != "0" ]
then
	echo "error - could not make output file"
	exit 1
fi
sleep 1

let step=$(( numsnps / granularity1 ))
if [[ $step -lt 1 ]]
then
	let step=1
fi

let count=0
let ycount=0
let xedge=0
let yedge=0
for (( y = 1; y <= $numsnps; y += $step ))
do
        let ycount=$(( ycount + 1 ))
        if [[ $ycount -ge $granularity1 ]]
        then
		let yedge=1
        fi
	let xcount=0
	for (( x = 1; x <= $numsnps; x += $step ))
	do
		if [[ $x -gt $y ]]
		then
			break
		fi
		let xcount+=1
                if [[ $xcount -eq $granularity1 ]]
                then
                   let xedge=1
                fi
		let count=$(( count + 1 ))
		let snp1=$x
		let snp2=$y
                echo "running ccc.sh with granularity $granularity2 snp1 $snp1 snp2 $snp2 step $step count $count"
		command sbatch --output=$slurmoutfile ./ccc.sh $inputfile $outputfile $threshold $numind $numsnps $numheaderrows $numheadercols $granularity2 $maxprocesses $outputfolder $snp1 $snp2 $step $count $xedge $yedge
	done
        if [[ $ycount -ge $granularity1 ]]
        then
           break
        fi
done

