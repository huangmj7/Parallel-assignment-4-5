#!/bin/sh

pwd="/gpfs/u/home/PCP8/PCP8hnhh/scratch/hw45"

#------------------------------------test 3-----------------------------------
ticks=256
threshold=0.25
Nnode=64
total=$( ($Nnode * 64) )

#0 threads per MPI rank (64 MPI ranks per compute node)
nthread=1
nrank=$( ($total / $nthread) )
srun --ntasks	$nrank	--overcommit --cpus-per-task=${nthread}  -o R${nrank}_T${nthread}.log	${pwd}/main.xl $nthread 0 $ticks $threshold

#4 threads per MPI rank (16 MPI ranks per compute node, rest are threads)
nthread=4
nrank=$( ($total / $nthread) )
srun --ntasks	$nrank	--overcommit --cpus-per-task=${nthread}  -o R${nrank}_T${nthread}.log	${pwd}/main.xl $nthread 0 $ticks $threshold

#16 threads per MPI rank ( 4 MPI ranks per compute node, rest are threads)
nthread=16
nrank=$( ($total / $nthread) )
srun --ntasks	$nrank	--overcommit --cpus-per-task=${nthread} -o R${nrank}_T${nthread}.log	${pwd}/main.xl $nthread 0 $ticks $threshold

#32 threads per MPI rank ( 2 MPI ranks per compute node, rest are threads)
nthread=32
nrank=$( ($total / $nthread) )
srun --ntasks	$nrank	--overcommit --cpus-per-task=${nthread} -o R${nrank}_T${nthread}.log	${pwd}/main.xl $nthread 0 $ticks $threshold

#32 threads per MPI rank ( 2 MPI ranks per compute node, rest are threads)
nthread=64
nrank=$( ($total / $nthread) )
srun --ntasks	$nrank	--overcommit --cpus-per-task=${nthread} -o R${nrank}_T${nthread}.log	${pwd}/main.xl $nthread 0 $ticks $threshold
#----------------------------------end of test--------------------------------
