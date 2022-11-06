#!/bin/bash

#SBATCH -p Lewis
#SBATCH -n 1
#SBATCH --mem 32G
#SBATCH --output="carriers-%j.out"

srun /home/jjs3k2/BlocBuster/carriers/carriers /home/jjs3k2/data/47798.bfs /home/jjs3k2/data/47798transpose.txt /home/jjs3k2/data/47798transpose.txt 0 0 /home/jjs3k2/data/47798.info 11 1 315 315 47798 /home/jjs3k2/data/47798.carriers.out
