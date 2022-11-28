#!/bin/bash

# overwrite all global variables with your input values
# also set sbatch values in ccc.sh, bfs.sh, and carriers.sh
# also make sure that bloc.h is set to ROWS_R_SNPS=0 in mproc or ccc project folder
# and BlocBuster is compiled to return 0 as true rather than 1 for Linux responses
# then run with ./batch.sh or ./batch.sh > out.txt & to run in background
# find results in "data" folder

case_file="../data/TSI-8-102-1000" # path to file outside data folder
control_file="../data/TSI-8-102-1000" # path to file outside data folder
blocbuster_path="/home/jjs3k2/mproc/mproc/blocbuster" # path to executable file 
blocbuster_folder="log" # folder name without path, builds a temp folder within the data folder
bfs_path="/home/jjs3k2/BlocBuster/bfs/bfs" # path to executable file
carriers_path="/home/jjs3k2/BlocBuster/carriers/carriers" # path to executable file
gml_file="out.gml" # file name without path
bfs_file="out.bfs" # file name without path
carriers_file="carriers.out" # file name without path
snp_info_file="snps.info" # file name without path
delimiter=' ' # delimiter in case/control file
let num_cases=102
let num_controls=102
let num_snps=1000
let case_control_header_rows=1
let case_control_header_columns=11
let info_header_columns=$case_control_header_columns
let info_header_rows=$case_control_header_rows
threshold="0.7"
let granularity1=0
let granularity2=3
let maxProcs=6
let semaphores=0 # 0 = false, 1 = true, default 0 (recommended)

# no modification required after this line

# data cleaning
if [[ $num_snps -le $num_cases || $num_snps -le $num_controls ]]
then
	echo "error - fewer snps than individuals"
	exit 1
fi
tmp_dir="data"
rm -rf $tmp_dir
sleep 1
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
	echo "processing $infile"
	resultfile=$2
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
	cp $outfile4 $resultfile
	rm $outfile1 $outfile2 $outfile3 $outfile4
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

# mproc

infile=$case_file
outfile=$gml_file
let num_ind=$num_cases

if [[ $granularity1 -gt 0 ]]
then
	pidlist=`srun mproc1.sh $blocbuster_path $infile $outfile $threshold $num_ind $num_snps $case_control_header_rows $case_control_header_columns $granularity1 $granularity2 $maxProcs $blocbuster_folder $semaphores`
	out=${pidlist#*Submitted batch job}
	ids=`echo $out | sed "s/Submitted batch job//g"`
	#pidlist=`echo $ids | sed "s/ /,afterok:/g"`
	pids=()
	for id in "${ids[@]}"
	do
		#echo $id
		pids+=($id)
	done
	pidlist=`echo $ids | tr ' ' ','`
	echo "running sbatch $pidlist"
else
	echo "************************************"
	echo "running mproc"
	bash virt_mproc1.sh $blocbuster_path $infile $outfile $threshold $num_ind $num_snps $case_control_header_rows $case_control_header_columns $granularity1 $granularity2 $maxProcs $blocbuster_folder $semaphores
fi

if [[ $granularity1 -gt 0 ]]
then
	echo "waiting for processes to complete"
	for p in ${pids[@]}
	do
		params="sacct -j $p --format=state"
		#echo $params
		stats=`$params`
		let stat=`echo $stats | grep "COMPLETED" | wc -l`
		#echo $stat
		let stat2=`echo $stats | grep "FAILED" | wc -l`
		while [[ $stat -eq 0 && $stat2 -eq 0 ]]
		do
			sleep 1
			stats=`$params`
			let stat=`echo $stats | grep "COMPLETED" | wc -l`
			let stat2=`echo $stats | grep "FAILED" | wc -l`
			#echo $stat
		done
		if [[ $stat2 -ne 0 ]]
		then
			echo "process $p failed"
			exit 1
		fi
		echo "process $p complete"
done
fi

# mproc 

#params="--dependency=afterok:$pidlist mproc2.sh"
#echo $params
if [[ $granularity1 -gt 0 ]]
then
	params="mproc2.sh $blocbuster_path"
	x=`srun $params`
	y=`echo $x | sed "s/Submitted batch job//g"`
	pid=`echo $y | sed "s/ //g"`
	echo "running sbatch $pid"
	pid1=$pid
else
	params="virt_mproc2.sh $blocbuster_path"
	echo "************************************"
	echo "running mproc -z"
	bash $params
fi

# bfs

infile="$tmp_dir/$outfile"
outfile="$tmp_dir/$bfs_file"

if [[ $granularity1 -gt 0 ]]
then
	params="--parsable --dependency=afterok:$pid bfs.sh $bfs_path $infile $outfile"
	pid=`sbatch $params`
	echo "running sbatch $pid"
	pid2=$pid
else
	echo "************************************"
	echo "running bfs"
	params="virt_bfs.sh $bfs_path $infile $outfile"
	bash $params
fi

# carriers

infile=$outfile
outfile="$tmp_dir/$carriers_file"

if [[ $granularity1 -gt 0 ]]
then
	pid=`sbatch --parsable --dependency=afterok:$pid carriers.sh $carriers_path $infile $case_file $control_file $case_control_header_rows $case_control_header_columns $snp_info_file $info_header_columns $info_header_rows $num_cases $num_controls $num_snps $outfile`
	echo "running sbatch $pid"
	pid3=$pid
else
	echo "************************************"
	echo "running carriers"
	bash virt_carriers.sh $carriers_path $infile $case_file $control_file $case_control_header_rows $case_control_header_columns $snp_info_file $info_header_columns $info_header_rows $num_cases $num_controls $num_snps $outfile
fi

if [[ $granularity1 -gt 0 ]]
then
	echo "waiting for processes to complete"
	stats=`sacct -j $pid --format=state`
	let stat=`echo $stats | grep "COMPLETED" | wc -l`
	let stat2=`echo $stats | grep "COMPLETED" | wc -l`
	while [[ $stat -eq 0 && $stat2 -eq 0 ]]
	do
		sleep 1
		stats=`sacct -j $pid --format=state`
		let stat=`echo $stats | grep "COMPLETED" | wc -l`
		let stat2=`echo $stats | grep "COMPLETED" | wc -l`
	done
	echo "$pid1,$pid2,$pid3 complete"
else
	echo "************************************"
	echo "complete"
	echo "************************************"
fi

