#!/bin/bash

#SBATCH -p Lewis
#SBATCH -n 1
#SBATCH --mem 4G
#SBATCH --output="install-BlocBuster-%j.out"

srun ./install
