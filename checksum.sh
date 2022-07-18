#!/bin/bash

let x1=0
let y1=0
let x2=48
let y2=48

for (( i = $x1; i < $x2; ++i ))
do
	for (( j = $(( i + 1 )); j <= $y2; ++j ))
	do
		#if [[ $i -ge $j ]]
		#then
			#continue
		#fi
		s="i $i j $j "
		let z=$( grep "$s" out* | wc -l )
		if [[ $z -eq 0 ]]
		then
			echo "i $i j $j not found" 
		elif [[ $z -gt 1 ]]
		then
			echo "i $i j $j = $z"
		fi
	done
done
