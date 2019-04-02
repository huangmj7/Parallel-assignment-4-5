<h1>Homework 4 & 5</h1>

<h3>Unfinished Work</h3>
<p>
I have already finished the # of lives in the local universe. So, you need to finish the MPI_Reduce part. Note that the reduction need to be done before my reset function or the recorded data will be losted.
</p>

<h3>Compilation</h3>
<p>Please compile a binary file named main.xl.
</p>

<h3>Shell Script</h3>
<h4>Commandline Job submission</h4>
<p>
For run1.sh, it is a test for 4-node configuration. The commandline is:<br/>
sbatch --partition small --nodes 4 --time 60 ./run.sh<br/>
<br/>
For run2.sh, it is a test for 16-node configuration. The commandline is:<br/>
sbatch --partition small --nodes 16 --time 60 ./run.sh<br/>
<br/>
For run3.sh, it is a test for 64-node configuration. The commandline is:<br/>
sbatch --partition small --nodes 32 --time 60 ./run.sh<br/>
<br/>
For run4.sh, it is a test for 128-node configuration. The commandline is:<br/>
sbatch --partition small --nodes 64 --time 60 ./run.sh<br/>
<br/>
For run5.sh, it is a test for Parallel IO and Heatmap, the commandline is:<br/>
sbatch --partition medium --nodes 128 --time 30 ./run.sh<br/>
<br/>
</p>

<h4>Shell Script(IMPORTANT)</h4>
<p>
For all run script, change the pwd before running. To get pwd, you can use 'pwd' to get the hard path of current directory.<br/>
Before running run5.sh, change nthread. nthread is the best ratio in 128-node configuration.
</p>
