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

x_start=()
x_stop=()
y_start=()
y_stop=()
for (( x = 0; x < $granularity1; x += 1 ))
do
	for (( y = 0; y < $granularity1; y += 1 ))
	do
		if [[ $x -gt $y ]]
		then
			continue
		fi
		if [[ $x -eq $(( granularity1 - 1 )) ]]
		then
			x_start+=( $(( x * $step + 1 )) )
			x_stop+=($numsnps)
		else
			x_start+=( $(( x * $step + 1 )) )
			x_stop+=( $(( (x + 1) * $step + 1 )) )
		fi
		if [[ $y -eq $(( granularity1 - 1 )) ]]
		then
			y_start+=( $(( y * $step + 1 )) )
			y_stop+=( $((numsnps + 1)) )
		else
			y_start+=( $(( y * $step + 1 )) )
			y_stop+=( $(( (y + 1) * $step + 1 )) )
		fi
	done
done

let count=1
for (( x = 0; x < ${#x_start[@]}; x += 1 ))
do
	#echo "running ccc.sh with granularity $granularity2 step $step count $count x-start ${x_start[$x]} x-stop ${x_stop[$x]} y-start ${y_start[$x]} y-stop ${y_stop[$x]}"
	command sbatch --output=$slurmoutfile ./ccc.sh $inputfile $outputfile $threshold $numind $numsnps $numheaderrows $numheadercols $granularity2 $maxprocesses $outputfolder $count $step ${x_start[$x]} ${x_stop[$x]} ${y_start[$x]} ${y_stop[$x]}
	let count+=1
done

