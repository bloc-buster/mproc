#!/bin/bash

case_file=$1
control_file=$2
case_control_header_rows=$3
case_control_header_columns=$4
delimiter=$5
if [[ $delimiter == 'none' ]]
then
	delimiter=' '
fi
tmp_dir=$6
num_snps=$7
snp_info_file=$8
skip=$9

# data cleaning

tmp_file="tmp.txt"
tmp_file2="tmp2.txt"
tmp_file3="tmp3.txt"
tmp_file4="tmp4.txt"
outfile1="$tmp_dir/$tmp_file"
outfile2="$tmp_dir/$tmp_file2"
outfile3="$tmp_dir/$tmp_file3"
outfile4="$tmp_dir/$tmp_file4"

function clean(){
	if [[ $skip == "first" ]]
	then
		return
	fi
	#infile=$1
	echo "processing $infile"
	#outfile=$2
	# if have csv file, convert snps file to delimiters as blanks
	if [[ $delimiter != ' ' ]]
	then
		tr ',' ' ' < $infile > $outfile1
	else
		cp $infile $outfile1
	fi
	# copy header columns into separate info file
	if [[ $case_control_header_columns -ge 0 ]]
	then
		cut -d ' ' -f 1-$case_control_header_columns < $infile > $snp_info_file
	else
		touch $snp_info_file
	fi
	# remove all header rows/columns
	# remove header rows by writing all lines except top lines
	if [[ $case_control_header_rows -ge 0 ]]
	then
		tail -n $num_snps $outfile1 > $outfile2
	fi
	# remove header columns with --complement option
	if [[ $case_control_header_columns -ge 0 ]]
	then
		cut -d ' ' -f 1-$case_control_header_columns --complement < $outfile2 > $outfile3
	fi
	# file should now have columns > rows
	num_rows=`wc -l $outfile3`
	num_cols=`head -n 1 $outfile3 | wc -w`
	num_rows=${num_rows%% *}
	num_cols=${num_cols%% *}
	#echo "file has $num_rows rows and $num_cols columns"
	# if not, has snps as rows, transpose so that snps are columns (required by carriers)
	delim="none"
	printout="false"
	if [[ $num_rows -gt $num_cols ]]
	then
		#echo "transposing input file $infile"
		python3 transpose.py $outfile3 $delim $outfile4 $printout 
		num_rows=`wc -l $outfile4`
		num_cols=`head -n 1 $outfile4 | wc -w`
		num_rows=${num_rows%% *}
		num_cols=${num_cols%% *}
		#echo "file has $num_rows rows and $num_cols columns"
	else
		cp $outfile3 $outfile4
	fi
	cp $outfile4 $outfile
	rm $outfile1 $outfile2 $outfile3 $outfile4
}

infile=$case_file
outfile="$tmp_dir/case_file.txt"
clean
#clean $infile $outfile
echo "cleaned $infile"
case_file=$outfile
infile=$control_file
outfile="$tmp_dir/control_file.txt"
clean
#clean $infile $outfile
echo "cleaned $infile"

