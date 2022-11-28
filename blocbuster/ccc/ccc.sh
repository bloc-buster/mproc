#!/bin/bash

#SBATCH -p Lewis
#SBATCH -n 1
#SBATCH --mem 4G
#SBATCH --output="ccc-%j.out"

srun ./ccc example_10indiv_6snp.txt temp.gml 0.7 10 6 1 1

