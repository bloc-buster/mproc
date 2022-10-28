#!/bin/bash

#SBATCH -p Lewis
#SBATCH -n 1
#SBATCH --mem 32G
#SBATCH --output="/home/jjs3k2/data/hapmap-new/chr10-CHB-139-214061-%j.out"
#SBATCH --time=1-00:00:00
#SBATCH --account=climerlab

srun /home/jjs3k2/BlocBuster/ccc/ccc "/home/jjs3k2/data/hapmap-new/chr10-CHB-139-214061.txt" "/home/jjs3k2/data/hapmap-new/out/chr10-CHB-out.gml" 0.8 139 214061 1 11

