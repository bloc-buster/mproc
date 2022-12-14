## BLOCBUSTER
## written by Sharlee Climer
- (climer@mail.umsl.edu)
- BlocBuster computes blocks of SNP correlations from input case/control SNP files.
- Refer to instructions in the BlocBuster folder.

## requirements:
- g++ compiler
- make
- Linux

## blocbuster-pipeline
- The Blocbuster pipeline consists of ccc, keephi, bfs, then carriers.
- The Blocbuster pipeline has been scripted for you.
- The scripts require a SLURM HPC system.
- Just change the global variables at the top of batch.sh to match your configuration.
- And change the sbatch headers at the tops of the .sh files.
- Then run ./batch.sh or srun batch.sh.

## The remainder of this file refers to the Mproc program.

## MPROC
## written by James Smith
- (jjs3k2@umsystem.edu)
- Mproc is the batched multiprocessed version of the CCC module from the BlocBuster program. The original BlocBuster program is stored in the "BlocBuster" folder. To run the entire BlocBuster pipeline with Mproc, use Mproc in place of the CCC module (the first module of the pipeline), or use mproc-pipeline.

## requirements:
- g++ compiler
- make
- Python
- Linux
- SLURM (or virtual cloud instance with many processors)
- for sbatch parameters, either specify them as options to blocbuster, or overwrite mproc.sh and ccc.sh with sbatch parameters relevant to your system

## mproc-pipeline
- Mproc is an alternative version of the first module of BlocBuster.
- The Mproc pipeline consists of mproc, keephi, bfs, then carriers.
- The entire pipeline has been scripted for you.
- The scripts require either a SLURM HPC system or a Linux computing instance.
- Just change the global variables at the top of batch.sh to match your configuration.
- For SLURM, change the sbatch variables in batch.sh also.
- To run without an HPC system, set granularity1 = 0 in batch.sh.
- Also, in the mproc folder, in bloc.h, set ROWS_R_SNPS = 0 (snps as columns).
- Then, in the mproc-pipeline folder, run ./batch.sh or srun batch.sh.
- To run just the first half of the pipeline (the bottleneck), then vary the parameters for the second half of the pipeline, run:
- - ./batch.sh -a (for the first half of the pipeline)
- - ./batch.sh -z (for the second half of the pipeline)
- - reconfigure parameters in batch.sh, then run
- - ./batch.sh -z

## The remainder of this file is an explanation of the Mproc program.

## Mproc required files:
- blocbuster.cpp (invokes main.sh or ccc.sh)
- main.sh (invokes mproc.sh)
- mproc.sh (invokes mproc.cpp)
- mproc.cpp (invokes helper.cpp)
- helper.cpp (a single process)
- ccc.sh (invokes ccc)
- bloc.cpp (alias for ccc)
- sem.h (semaphores)
- shmem.h (shared memory)
- timer.h (log output from ccc)
- params.h (blocbuster.cpp configuration file)
- bloc.h (bloc.cpp configuration file)
- checksum.txt (generated programmatically)
- data.key (generated programmatically)
- Makefile
- **alternative files for non-HPC system**
- virt_main.sh
- virt_mproc.sh
- virt_ccc.sh

## mproc pipelines
- the following is an explanation of the flow logic for the files in the mproc folder
- job scheduling and multiprocessing on HPC system:
- - blocbuster.cpp => main.sh => mproc.sh => mproc.cpp => helper.cpp
- job scheduling without multiprocessing on HPC system:
- - blocbuster.cpp => main.sh => ccc.sh => bloc.cpp
- multiprocessing without job scheduling on non-HPC computer:
- - blocbuster.cpp => virt_main.sh => virt_mproc.sh => mproc.cpp => helper.cpp
- job scheduling without multiprocessing on non-HPC computer:
- - blocbuster.cpp => virt_main.sh => virt_ccc.sh => bloc.cpp

## project structure:
- The Mproc program is stored in the mproc folder. We require that all output be stored outside of the mproc folder. 
- The start file of Mproc is named blocbuster, although Mproc represents just the first module of the BlocBuster pipeline. After running Mproc, BlocBuster still requires that you run bfs and carriers to complete the pipeline. 
- The output file is automatically placed in the same folder as the input file, and the output folder containing log files will be placed in that folder as well. Any attempt to read from or write to the folder containing the executables will result in an error.
- project folder (name of your choice)
- - BlocBuster (folder)
- - blocbuster-pipeline (folder)
- - - SLURM scripts for a full run of the BlocBuster pipeline
- - mproc (folder)
- - - scripts and executable files
- - mproc-pipeline (folder)
- - - scripts for a full run of the mproc pipeline on either a SLURM HPC system or a Linux cloud instance
- - README.md (text file)
- - data (folder)
- - - example file with 1000 snps, 102 individuals, 1 header rows, 11 header columns

## configure:
- bloc.h has been set for SNPs as columns 
- if you need SNPs as rows, change ROWS_R_SNPS setting in bloc.h
- for sbatch settings, either overwrite mproc.sh and ccc.sh with appropriate sbatch parameters, or specify options to blocbuster 

## compile (from within mproc folder):
- srun make clean
- srun make
- (on each run, main.sh recompiles several files since it must rewrite params.h)

## run:
- the mproc program requires two runs
- - on the first run, the program generates partial files in the output folder
- - on the second run, the program gathers the partial files into a complete file
- first, run blocbuster with srun blocbuster or just ./blocbuster
- wait for all jobs to complete
- then, run blocbuster with -z and no parameters
- read the SLURM output file from the most recent job to verify the checksum
- - if observed comparisons equals expected comparisons then the results are correct

## different runs at same time:
- not possible - you must wait until each batch completes before starting another run
- after the first batch completes, you must perform the second run with ./blocbuster -z
- the -z option will read the params.h file containing the starting input parameters
- if you run blocbuster.cpp again on new files before running blocbuster -z on the first files, the params.h variables will be overwritten
- hence, you must run the program on one batch of files at a time
- a large file should not take much longer than an hour so waiting on each run should not take long

## usage:
- **prepare files**
- ./blocbuster input.txt output.gml threshold numInd numSNPs numHeaderRows numHeaderCols granularity1 (default 1) granularity2 (default 7) max_simultaneous_processes (default 15) temp_output_folder (default log_files) semaphores (default 0) 
- - explanation - the temp_output_folder is not for the output.gml file, it's a temporary folder that stores partial gml files which are erased after the -z option
- **combine files**
- ./blocbuster -z
- - explanation - program does not know when all jobs have finished...after all jobs complete, tell program to join partial files from output folder into a gml file

## parameters:
- input - a snps data file containing snp data
- output - name for the output gml file without path, the path is autogenerated and the file will appear next to the input file, any file with same name will be overwritten
- threshold - decimal between 0 and 1
- numInd - number of individuals in the input file
- numSNPs - number of snps in the input file
- numHeaderRows - number of header rows at top of input file
- numheaderCols - number of non-data columns at front of each row
- (the rest of the parameters have defaults and may be left blank)
- granularity1 - integer for number of divisions into start table made by blocbuster.sh
- - optionally 0...for non-hpc system, the program will not schedule SLURM batch processing, but will run multiprocessing
- granularity2 - integer for number of divisions into those divisions made by each run
- - optionally 0...the program will not run multiprocessing, instead just splitting the file into partitions and launching a separate SLURM job on each
- max_simultaneous_processes - the number of processors found in each node on your HPC system, or the maximum processes at a time allowed on your system
- temp_output_files - name for a temporary folder without path, not for the output.gml file, small partial files are written to the output folder, the folder is autogenerated and will appear next to the input file, any folder with same name will be removed
- semaphores - either 0 or 1, default 0, if 1 the program will reduce the number of output files by having multiple processes write to the same file which will be locked with semaphores, if 0 the program will let each process write to a separate file which will a large number of small files in the temp output folder

## options:
- The parameters above are positional until the granularity parameters, after which the parameters may be specified either positionally or with the following options.
- Options should be written before all other parameters except the filename.
- - e.g. ./mproc/blocubster -P hpc3 -M 32G -T 0-01:00:00 intpufile outputfile 0.8 100 1000 1 11
- options:
- *-g value* (granularity 1) 
- *-G value* (granularity 2) 
- *-h* (help) 
- *-o value* (output folder name) 
- *-p value* (max processes) 
- *-s value* (semaphores, 0 yes 1 no) 
- *-P value* (slurm partition)
- *-M value* (slurm memory)
- *-T value* (slurm max time)
- *-z* (second run)
- *-n* (quantify resulting processes or jobs) 

## output:
- .gml file with results, do not put in the temp output folder
- log_files folder (erasable folder containing partial gml files)
- checksum files in log_files folder
- partial gml files in log_files folder
- SLURM output files in log_files folder (ends with .out)

## example run:
- the example file from the data folder has 1000 snps and 102 individuals
- however, it has SNPs as rows
- change bloc.h to ROWS_R_SNPS = 1, then run
- ./mproc/blocbuster ./data/example_102_1000.txt out.gml 0.7 102 1000 1 11 3 3 10 temp_output_files
- (wait until all jobs have completed, then generate gml file)
- ./mproc/blocbuster -z

## checking results:
- after invoking -z, read the resulting SLURM output file (with the highest number)
- if the expected comparisons don't equal the comparisons, then the resulting gml file is wrong
- your selected granularity values may not have been a good fit for the input file size
- run the entire program again with lower granularity values

## troubleshooting:
- if the program ever throws an error, check if it forgot to release memory or semaphores
- type
- ipcs
- then, if your username shows up in the list, memory or semaphores were not released
- so, find the id that appears next to your username
- if your name appeared in the memory section, type
- ipcrm -m #
- where # = id
- if your name appeared in the semaphore section, type
- ipcrm -s #
- where # = id
- then, type ipcs again to make sure it was released

## disclaimer:
- We are not responsible for any damages resulting from use of the product. Email one of the authors if you have questions.

