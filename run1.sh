#!/bin/sh

pwd="/gpfs/u/home/PCP8/PCP8hnhh/scratch/hw45"

#------------------------------------test 1-----------------------------------

total=256
nrank=256
nthread=$(($total / $nrank))
srun --ntasks	$nrank	--overcommit --cpus-per-task=1 -o R${nrank}_T${nthread}.log	${pwd}/main.xl $nthread 

nrank=64
nthread=$(($total / $nrank))
srun --ntasks	$nrank	--overcommit --cpus-per-task=4 -o R${nrank}_T${nthread}.log	${pwd}/main.xl $nthread 

nrank=16
nthread=$(($total / $nrank))
srun --ntasks	$nrank	--overcommit --cpus-per-task=16 -o R${nrank}_T${nthread}.log	${pwd}/main.xl $nthread 

nrank=8
nthread=$(($total / $nrank))
srun --ntasks	$nrank	--overcommit --cpus-per-task=32 -o R${nrank}_T${nthread}.log	${pwd}/main.xl $nthread 

nrank=4
nthread=$(($total / $nrank))
srun --ntasks	$nrank	--overcommit --cpus-per-task=64 -o R${nrank}_T${nthread}.log	${pwd}/main.xl $nthread 
#----------------------------------end of test--------------------------------
