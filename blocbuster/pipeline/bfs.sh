#!/bin/bash

#SBATCH -p Lewis
#SBATCH -n 1
#SBATCH --mem 32G
#SBATCH --output="bfs-%j.out"

srun /home/jjs3k2/BlocBuster/bfs/bfs /home/jjs3k2/data/v8/47798raw3.gml /home/jjs3k2/data/v8/47798raw3.bfs

