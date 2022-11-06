#!/bin/bash

#SBATCH -p Lewis
#SBATCH -n 1
#SBATCH --mem 32G
#SBATCH --output="transpose-%j.out"

srun python3 transpose.py /home/jjs3k2/data/47798raw2.txt " " /home/jjs3k2/data/47798transpose.txt false
