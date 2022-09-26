#!/bin/bash

# get absolute path of main.sh
root=$0
# build path for next file
if grep -q "/" <<< $root
then
	# if $0 is a path then remove main.sh and / from path
	runpath=${root%/*}
else
	# if $0 is just a file name without a forward slash then change working directory to .
	runpath="."
fi
# if second run, build command for batch script mproc.sh
if [[ "$#" -eq 1 ]]
then
	slurmoutfile=$1
	params="$runpath/virt_mproc.sh $runpath" 
	command $params
	exit 0
# otherwise validate number of args
elif [[ "$#" -lt 7 || "$#" -gt 13 ]]
then
	echo "$#"
	echo "usage: ./main.sh input.txt output.gml threshold numInd numSNPs numHeaderRows numHeaderCols granularity1 (default 1) granularity2 (default 7) max_simultaneous_processes (default 15) temp_output_folder (default log_files)"
	echo "alternate (only for conclude): ./main.sh slurm-output-file-name"
	exit 1
fi
# read command line args (from blocbuster.cpp)
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
slurmoutfile="$outputfolder%j.out" # not used
let semaphores=0
if [ $numargs -ge 12 ]
then
	semaphores=$1
	shift
fi
# build params.h file that will save absolute paths and variables for second run
params="
#define num_ind $numind\n
#define num_snps1 $numsnps\n
#define num_snps2 $numsnps\n
#define granularity1 $granularity1\n
#define granularity2 $granularity2\n
#define semaphores $semaphores\n
#define gml_file \"$outputfile\"\n
#define temp_folder \"$outputfolder\"\n
#define root $runpath/\n
#define DATAKEYNAME \"$runpath/data.key\"\n
"
# save working directory prior to changing to directory of executables
wd=$( pwd )
command cd $runpath
# save params.h into same folder as executables
command echo -e $params > params.h
# recompile mproc.cpp and helper.cpp, which import params.h as a dependency and require the latest settings
sleep 1
command make clean
sleep 1
command make mproc
sleep 1
command make helper
# check status of recompilations
let status=$?
if [ "$status" != "0" ]
then
	echo "compilation error - exiting"
	exit 1
fi
# change back to working directory that program was run from
command cd $wd
# remove previous output folder
command rm -rf $outputfolder
sleep 1
# make new output folder
command mkdir $outputfolder
# check status of new output folder
let status=$?
if [ "$status" != "0" ]
then
	echo "error - could not make output folder"
	exit 1
fi
sleep 1
# make empty output file
command touch $outputfile
# check status of output file
let status=$?
if [ "$status" != "0" ]
then
	echo "error - could not make output file"
	exit 1
fi
sleep 1

# modify for non-hpc system
let granularity1=1

# begin building SNP partitions for batch processing
# compute batch processing step value
let step=$(( numsnps / granularity1 ))
# do not allow zero or negative step values
if [[ $step -lt 1 ]]
then
	let step=1
fi
# arrays for indices of SNP partitions
x_start=()
x_stop=()
y_start=()
y_stop=()
# compute indices of SNP partitions starting from 1 for easy validation (C++ files must subtract one from them)
# in a SNPs x SNPs matrix, x and y are the same
# for each SNP x, for each SNP y, add SNP pair to arrays unless x and y are the same SNP or have already added them with x and y reversed
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
# launch a batch script for each partition
let count=1
for (( x = 0; x < ${#x_start[@]}; x += 1 ))
do
	# if multiprocessing, invoke mproc.sh
	if [[ $granularity2 -gt 0 ]]
	then
		args="$runpath/virt_mproc.sh $inputfile $outputfile $threshold $numind $numsnps $numheaderrows $numheadercols $granularity2 $maxprocesses $outputfolder $count $step ${x_start[$x]} ${x_stop[$x]} ${y_start[$x]} ${y_stop[$x]} $runpath"
		command $args
	# if only batch processing (not used), invoke ccc.sh
	else
		command "$runpath/virt_ccc.sh" $inputfile $outputfile $threshold $numind $numsnps $numheaderrows $numheadercols $outputfolder $count ${x_start[$x]} ${x_stop[$x]} ${y_start[$x]} ${y_stop[$x]} $runpath
	fi
	# invoke partition counter
	let count+=1
done

