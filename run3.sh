#!/bin/sh


#------------------------------------test 3-----------------------------------
nrank=64
nthread=0
srun --ntasks	$nrank	--overcommit -o R${nrank}_T${nthread}.log	/gpfs/u/home/PCP8/PCP8hnhh/barn/hw45/main.xl $nthread

nthread=4
srun --ntasks	$nrank	--overcommit -o R${nrank}_T${nthread}.log	/gpfs/u/home/PCP8/PCP8hnhh/barn/hw45/main.xl $nthread

nthread=16
srun --ntasks	$nrank	--overcommit -o R${nrank}_T${nthread}.log	/gpfs/u/home/PCP8/PCP8hnhh/barn/hw45/main.xl $nthread

nthread=32
srun --ntasks	$nrank	--overcommit -o R${nrank}_T${nthread}.log	/gpfs/u/home/PCP8/PCP8hnhh/barn/hw45/main.xl $nthread

nthread=64
srun --ntasks	$nrank	--overcommit -o R${nrank}_T${nthread}.log	/gpfs/u/home/PCP8/PCP8hnhh/barn/hw45/main.xl $nthread
#----------------------------------end of test--------------------------------
