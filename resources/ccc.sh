#!/bin/bash

#SBATCH -p hpc3 
#SBATCH --mem 64G
#SBATCH --time=0-01:00:00

#echo "ERROR! Overwrite ccc.sh with sbatch parameters for your local system, then comment out these lines."
#exit 1

# if second run
if [[ "$#" -eq 2 ]]
then
	# combine partial gml files into single gml file
	tempfolder=$1
	shift
	gmlfile=$1
	shift
	files=$( ls $tempfolder )
	let firstfile=0
	for f in ${files[@]}
	do
		if [[ ${f##*\.} != "gml" ]]
		then
			continue
		fi
		file="$tempfolder$f"
		if [[ $firstfile -eq 0 ]]
		then
			text=$( head -n -1 "$file" )
			let firstfile+=1
		elif [[ $( grep edge "$file" ) == "" ]]
		then
			continue
		else
			x=$( grep -n edge "$file" | head -n 1 )
			x=${x%%:*}
			y=$( wc -l $file )
			y=${y%% *}
			let z=$(( y - x + 1 ))
			text=$( tail -n $z "$file" | head -n -1 )
		fi
		echo -e "$text" >> $gmlfile
	done
	echo "]" >> $gmlfile
	# remove partial gml files in temp folder
	for f in ${files[@]}
	do
		if [[ ${f##*\.} == "gml" ]]
		then
			file="$tempfolder$f"
			command rm $file
		fi
	done
	exit 0
# otherwise validate number of args
elif [[ "$#" -ne 14 ]]
then
	echo "args $#"
	echo "usage: ./ccc.sh input.txt output.gml threshold numInd numSNPs numHeaderRows numHeaderCols output_folder (default temp_output_files) count x1 x2 y1 y2"
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
outputfolder=$1
shift
# partial gml file count
let count=$1
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
# build name for partial gml file
# remove .gml extension from output gml file, append number, then reappend .gml
# problem if output file does not have .gml extension
outputfile=$outputfolder
outputfile+=$count
outputfile+=".gml"
# run partial blocbuster
srun "$runpath/ccc" $inputfile $outputfile $threshold $numind $numsnps $numheaderrows $numheadercols $xstart $xstop $ystart $ystop

