#!/bin/bash

runfile=$0
#echo "main.sh file name $runfile"
if grep -q "/" <<< $runfile
then
	runpath=${runfile%/*}
else
	runpath="."
fi
#echo "runpath $runpath"

if [[ "$#" -eq 1 ]]
then
	slurmoutfile=$1
	params="--output=$slurmoutfile%j.out $runpath/mproc.sh $runpath" 
	#echo "main.sh running sbatch $params"
	command sbatch $params
	#command sbatch --output="$slurmoutfile%j.out" "$runpath/mproc.sh" $runpath 
	exit 0
elif [[ "$#" -lt 7 || "$#" -gt 13 ]]
then
	echo "$#"
	echo "usage: ./main.sh input.txt output.gml threshold numInd numSNPs numHeaderRows numHeaderCols granularity1 (default 1) granularity2 (default 7) max_simultaneous_processes (default 15) temp_output_folder (default log_files)"
	echo "alternate (only for conclude): ./main.sh true"
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
let granularity1=1
if [ $numargs -ge 8 ]
then
	let granularity1=$1
	shift
fi
let granularity2=7
if [ $numargs -ge 9 ]
then
	let granularity2=$1
	shift
fi
let maxprocesses=15
if [ $numargs -ge 10 ]
then
	let maxprocesses=$1
	shift
fi
outputfolder="../log_files"
if [ $numargs -ge 11 ]
then
	outputfolder=$1
	shift
fi
slurmoutfile="$outputfolder%j.out"
let semaphores=0
if [ $numargs -ge 12 ]
then
	semaphores=$1
	shift
fi

params="
#define num_ind $numind\n
#define num_snps1 $numsnps\n
#define num_snps2 $numsnps\n
#define granularity2 $granularity2\n
#define semaphores $semaphores\n
#define gml_file \"$outputfile\"\n
#define temp_folder \"$outputfolder\"\n
#define runfile $runpath/\n
#define DATAKEYNAME \"$runpath/data.key\"\n
"
wd=$( pwd )
command cd $runpath
command echo -e $params > params.h
sleep 1
command srun make clean
sleep 1
command srun make mproc
sleep 1
command srun make helper
sleep 1
#command srun make ccc
#sleep 1
command cd $wd
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
for (( x = 1; x <= $granularity1; x += 1 ))
do
	for (( y = 1; y <= $granularity1; y += 1 ))
	do
		if [[ $x -gt $y ]]
		then
			continue
		fi
		if [[ $x -eq $(( granularity1 )) ]]
		then
			x_start+=( $(( (x-1) * $step + 1)) )
			x_stop+=( $numsnps )
		else
			x_start+=( $(( (x-1) * $step + 1)) )
			x_stop+=( $(( x * $step)) )
		fi
		if [[ $y -eq $(( granularity1 )) ]]
		then
			y_start+=( $(( (y-1) * $step + 1)) )
			y_stop+=( $numsnps )
		else
			y_start+=( $(( (y-1) * $step + 1)) )
			y_stop+=( $(( y * $step)) )
		fi
	done
done

let count=1
for (( x = 0; x < ${#x_start[@]}; x += 1 ))
do
	if [[ $granularity2 -gt 0 ]]
	then
		#echo "main.sh running mproc.sh with granularity $granularity2 step $step count $count x-start ${x_start[$x]} x-stop ${x_stop[$x]} y-start ${y_start[$x]} y-stop ${y_stop[$x]}"
		#echo "parameters $slurmoutfile $inputfile $outputfile $threshold $numind $numsnps $numheaderrows $numheadercols $granularity2 $maxprocesses $outputfolder $count $step ${x_start[$x]} ${x_stop[$x]} ${y_start[$x]} ${y_stop[$x]}"
		args="--output=$slurmoutfile $runpath/mproc.sh $inputfile $outputfile $threshold $numind $numsnps $numheaderrows $numheadercols $granularity2 $maxprocesses $outputfolder $count $step ${x_start[$x]} ${x_stop[$x]} ${y_start[$x]} ${y_stop[$x]} $runpath"
		#echo "main.sh running sbatch $args"
		command sbatch $args
	else
		command sbatch --output=$slurmoutfile "$runpath/ccc.sh" $inputfile $outputfile $threshold $numind $numsnps $numheaderrows $numheadercols $outputfolder $count ${x_start[$x]} ${x_stop[$x]} ${y_start[$x]} ${y_stop[$x]} $runpath
	fi
	let count+=1
done

