#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#	8. time spent waiting for locks
#
# output:
#	lab2b_1.png  ... throughput and number of threads
#	lab2b_2.png  ... the wait-for-lock time and the average time per operation against the number of competing threads
#	lab2b_3.png  ... threads and iterations that run (protected) w/o failure
#	lab2b_4.png  ... aggregated throughput vs. the number of threads mutex lock
#	lab2b_5.png  ... aggregated throughput vs. the number of threads spin lock
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

set title "Throughput vs. Number of Threads"
set xlabel "Number of Threads"
set logscale x 2
set ylabel "Throughput"
set logscale y 10
set output 'lab2b_1.png'
plot \
     "< grep list-none-m lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'mutex synchronized list operations' with points lc rgb 'violet', \
     "< grep list-none-s lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'spin-lock synchronized list operations' with points lc rgb 'blue'

set title "Wait-for-Lock time and the Average Time Per Operation vs. The Number of Competing Threads"
set xlabel "Number of threads"
set logscale x 2
set ylabel "Wait-for-Lock Time and Average Time per Operation"
set logscale y 10
set output 'lab2b_2.png'
plot \
     "< grep list-none-m lab2b_list.csv" using ($2):($7) \
	title 'average time per operation' with points lc rgb 'green', \
     "< grep list-none-m lab2b_list.csv" using ($2):($8) \
	title 'average wait-for-mutex time' with points lc rgb 'blue'
	
set title "Successful Iterations for Each Synchronization Method"
set xlabel "Number of Threads"
set logscale x 2
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'
plot \
     "< grep list-id-none lab2b_list.csv" using ($2):($3) \
	title 'unprotected (no synchronization)' with points lc rgb 'green', \
     "< grep list-id-s lab2b_list.csv" using ($2):($3) \
	title 'spin lock synchronization' with points lc rgb 'blue', \
	 "< grep list-id-s lab2b_list.csv" using ($2):($3) \
	title 'mutex synchronization' with points lc rgb 'red'
		
set title "Aggregated Throughput vs. The Number of Threads using Mutex lock w/ sublists"
set xlabel "Number of Threads"
set logscale x 2
set ylabel "Throughput"
set logscale y 10
set output 'lab2b_4.png'
plot \
     "< grep 'list-none-m,.*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '1 list' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,.*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '4 lists' with linespoints lc rgb 'green', \
     "< grep 'list-none-m,.*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '8 lists' with linespoints lc rgb 'blue', \
     "< grep 'list-none-m,.*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '16 lists' with linespoints lc rgb 'orange'

set title "Aggregated Throughput vs. The Number of Threads using Spin lock w/ sublists"
set xlabel "Number of Threads"
set logscale x 2
set ylabel "Throughput"
set logscale y 10
set output 'lab2b_5.png'
plot \
     "< grep 'list-none-s,.*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '1 list' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,.*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '4 lists' with linespoints lc rgb 'green', \
     "< grep 'list-none-s,.*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '8 lists' with linespoints lc rgb 'blue', \
     "< grep 'list-none-s,.*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '16 lists' with linespoints lc rgb 'orange'