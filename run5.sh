#!/bin/sh

pwd="/gpfs/u/home/PCP8/PCP8hnhh/scratch/hw45"

#------------------------------------test 4-----------------------------------
#For this part, record the time of Parallel and store the universes generated.
ticks=128
Nnode=128
total=$( ($Nnode * 64) )

nthread=?
nrank=$( ($total / $nthread) )

threshold=0
srun --ntasks	$nrank	--overcommit --cpus-per-task=${nthread} -o R${nrank}T${nthread}.log	${pwd}/main.xl $nthread 1 $ticks $threshold

threshold=0.25
srun --ntasks	$nrank	--overcommit --cpus-per-task=${nthread} -o R${nrank}T${nthread}.log	${pwd}/main.xl $nthread 1 $ticks $threshold

threshold=0.50
srun --ntasks	$nrank	--overcommit --cpus-per-task=${nthread} -o R${nrank}T${nthread}.log	${pwd}/main.xl $nthread 1 $ticks $threshold

threshold=0.75
srun --ntasks	$nrank	--overcommit --cpus-per-task=${nthread} -o R${nrank}T${nthread}.log	${pwd}/main.xl $nthread 1 $ticks $threshold
#----------------------------------end of test--------------------------------
