#!/bin/bash
# latency_test.sh
# Originally sourced from OSADL: 
#  https://www.osadl.org/uploads/media/mklatencyplot.bash
# Lightly modified for our purposes.. this script is to be run via it's
# wrapper script runtest.
#----------------------------------------------------------------------
# This program is part of the source code released for the book
#  "Linux Kernel Programming"
#  (c) Author: Kaiwan N Billimoria
#  Publisher:  Packt
#  GitHub repository:
#  https://github.com/PacktPublishing/Linux-Kernel-Programming
# 
# For details, refer the book, Ch 11.
#----------------------------------------------------------------------
# Notes/Ref:
# a) http://www.osadl.org/Create-a-latency-plot-from-cyclictest-hi.bash-script-for-latency-plot.0.html
# b) Detailed slides on cyclictest, good for understanding latency and it's
# measurement: 'Using and Understanding the Real-Time Cyclictest Benchmark',
# Rowand, Oct 2013: https://events.static.linuxfound.org/sites/events/files/slides/cyclictest.pdf
name=$(basename $0)

[ $# -ne 1 ] && {
 echo "Usage: ${name} \"<test title>\""
 exit 1
}
title="$1"
[ ${#title} -gt 25 ] && {
 echo "${name}: title string passed too long (max len is 25 chars)"
 exit 1
}

which cyclictest >/dev/null && pfx="" || {
pfx=~/rtl_llkd/rt-tests/                    # adjust as required !
 [ ! -x ${pfx}/cyclictest ] && {
   echo "${name}: cyclictest not located, aborting..."
   exit 1
 }
}

echo "--------------------------"
echo "Test Title :: \"${title}\""
echo "--------------------------"
echo "Version info:"
lsb_release -a
uname -a
cat /proc/version
echo

# 1. Redirect the output of cyclictest to a file, for example
loops=100000000
# (Please note that this with loops==100,000,000 will take 5 hours and 33 minutes.)
# alternatively: run cyclictest by duration
#duration=12h
duration=1h
echo "sudo ${pfx}cyclictest --duration=${duration} -m -Sp90 -i200 -h400 -q >output"
sudo ${pfx}cyclictest --duration=${duration} -m -Sp90 -i200 -h400 -q >output

# 2. Get maximum latency
min=$(grep "Min Latencies" output | tr " " "\n" | grep "^[0-9]" | sort -n | head -1 | sed s/^0*//)
max=$(grep "Max Latencies" output | tr " " "\n" | sort -n | tail -1 | sed s/^0*//)
avg=$(grep "Avg Latencies" output | tr " " "\n" | grep "^[0-9]" | sed s/^0*// |awk '{sum += $1} END {print sum/NR}')
latstr="min/avg/max latency: ${min} us / ${avg} us / ${max} us"
echo "${latstr}"

# 3. Grep data lines, remove empty lines and create a common field separator
grep -v -e "^#" -e "^$" output | tr " " "\t" >histogram 

# 4. Set the number of cores, for example
#cores=4
# (If the script is used on a variety of systems with a different number of cores,
# this can, of course, be determined from the system.)
cores=$(nproc)

# 5. Create two-column data sets with latency classes and frequency values for each core
for i in $(seq 1 $cores)
do
    column=`expr $i + 1`
    cut -f1,$column histogram >histogram$i
done

# 6. Create plot command header
title="${title}: ${latstr} ; kernel: $(uname -r)"
echo -n -e "set title \"${title}\"\n\
    set terminal png size 800,480\n\
    set xlabel \"Latency (us): min $min us, avg $avg us, max $max us\"\n\
    set logscale y\n\
    set xrange [0:400]\n\
    set yrange [0.8:*]\n\
    set ylabel \"Number of latency samples\"\n\
    set output \"plot_$(uname -r).png\"\n\
    plot " >plotcmd

# 7. Append plot command data references
for i in $(seq 1 $cores)
do
      if test $i != 1
        then
	    echo -n ", " >>plotcmd
        fi
        cpuno=`expr $i - 1`
        if test $cpuno -lt 10
        then
          title=" CPU$cpuno"
        else
          title="CPU$cpuno"
        fi
        echo -n "\"histogram$i\" using 1:2 title \"$title\" with histeps" >>plotcmd
done

# 8. Execute plot command
gnuplot -persist <plotcmd
