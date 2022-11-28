#!/bin/bash

# overwrite all global variables with your input values
# also set sbatch values in ccc.sh, bfs.sh, and carriers.sh
# then run with ./batch.sh

case_file="/home/jjs3k2/pipeline/queue/in/TSI-8-102-1000.txt"
control_file="/home/jjs3k2/pipeline/queue/in/TSI-8-102-1000.txt"
delimiter=','
let num_cases=102
let num_controls=102
let num_snps=1000
let case_control_header_rows=1
let case_control_header_columns=11
gml_file="out.gml"
bfs_file="out.bfs"
carriers_file="carriers.out"
snp_info_file="snps.info"
let info_header_columns=$case_control_header_columns
let info_header_rows=$case_control_header_rows
threshold="0.8"

# no modification required after this line

# data cleaning
tmp_dir="./data"
mkdir $tmp_dir
tmp_file="tmp.txt"
tmp_file2="tmp2.txt"
tmp_file3="tmp3.txt"
tmp_file4="tmp4.txt"
outfile1="$tmp_dir/$tmp_file"
outfile2="$tmp_dir/$tmp_file2"
outfile3="$tmp_dir/$tmp_file3"
outfile4="$tmp_dir/$tmp_file4"
snp_info_file2="$tmp_dir/$snp_info_file"
snp_info_file=$snp_info_file2

function clean(){
	infile=$1
	resultfile=$2
	# if have csv file, convert snps file to delimiters as blanks
	if [[ $delimiter != ' ' ]]
	then
		srun tr ',' ' ' < $infile > $outfile1
	else
		cp $infile $outfile1
	fi
	# copy header columns into separate info file
	if [[ $case_control_header_columns -ge 0 ]]
	then
		srun cut -d ' ' -f 1-$case_control_header_columns < $infile > $snp_info_file
	else
		touch $snp_info_file
	fi
	# remove all header rows/columns
	# remove header rows by writing all lines except top lines
	if [[ $case_control_header_rows -ge 0 ]]
	then
		srun tail -n $num_snps $outfile1 > $outfile2
	fi
	# remove header columns with --complement option
	if [[ $case_control_header_columns -ge 0 ]]
	then
		srun cut -d ' ' -f 1-$case_control_header_columns --complement < $outfile2 > $outfile3
	fi
	# file should now have columns > rows
	num_rows=`wc -l $outfile3`
	num_cols=`head -n 1 $outfile3 | wc -w`
	num_rows=${num_rows%% *}
	num_cols=${num_cols%% *}
	echo "file has $num_rows rows and $num_cols columns"
	# if not, has snps as rows, transpose so that snps are columns (required by carriers)
	delim="none"
	printout="false"
	if [[ $num_rows -gt $num_cols ]]
	then
		echo "transposing input file"
		srun python3 transpose.py $outfile3 $delim $outfile4 $printout 
		num_rows=`wc -l $outfile4`
		num_cols=`head -n 1 $outfile4 | wc -w`
		num_rows=${num_rows%% *}
		num_cols=${num_cols%% *}
		echo "file has $num_rows rows and $num_cols columns"
	else
		cp $outfile3 $outfile4
	fi
	cp $outfile4 $resultfile
}

infile=$case_file
outfile="$tmp_dir/case_file.txt"
clean $infile $outfile
case_file=$outfile
infile=$control_file
outfile="$tmp_dir/control_file.txt"
clean $infile $outfile
control_file=$outfile
let case_control_header_rows=0
let case_control_header_columns=0

# ccc

infile=$case_file
outfile="$tmp_dir/$gml_file"
let num_ind=$num_cases

pid=`sbatch --parsable ccc.sh $infile $outfile $threshold $num_ind $num_snps $case_control_header_rows $case_control_header_columns`
echo "running sbatch $pid"

# bfs

infile=$outfile
outfile="$tmp_dir/$bfs_file"

pid=`sbatch --parsable --dependency=afterok:$pid bfs.sh $infile $outfile`
echo "running sbatch $pid"

# carriers

infile=$outfile
outfile="$tmp_dir/$carriers_file"

pid=`sbatch --parsable --dependency=afterok:$pid carriers.sh $infile $case_file $control_file $case_control_header_rows $case_control_header_columns $snp_info_file $info_header_columns $info_header_rows $num_cases $num_controls $num_snps $outfile`
echo "running sbatch $pid"

# find results in outfile

