#!/bin/bash

carriers_path=$1
shift
bfs_file=$1
shift
case_file=$1
shift
control_file=$1
shift
case_control_header_rows=$1
shift
case_control_header_columns=$1
shift
snp_info_file=$1
shift
info_header_columns=$1
shift
info_header_rows=$1
shift
num_cases=$1
shift
num_controls=$1
shift
num_snps=$1
shift
out_file=$1
shift
srun $carriers_path $bfs_file $case_file $control_file $case_control_header_rows $case_control_header_columns $snp_info_file $info_header_columns $info_header_rows $num_cases $num_controls $num_snps $out_file

