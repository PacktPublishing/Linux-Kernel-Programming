#!/bin/bash
# plotter_prep.sh
# Book: Linux Kernel Programming, Kaiwan N Billimoria, Packt.
# Part of the ch8/slab4_actualsz_wstg_plot code.
#
# Script to help prepare the data file from kernel log.
# We assume that:
# a) sudo dmesg -C   ; was done prior to the kernel module being inserted
#    (clearing any stale messages from the kernel log)
# b) the cut(1) below gets rid of the dmesg timestamp; we assume it's
#    (timestamp) there
# c) you will comment out or delete any extraneous lines in the final o/p
#    file 2plotdata.txt after this :-)
# (To save you the trouble, we've (also) kept the 2plotdata.txt file in the repo)
dmesg > /tmp/plotdata
cut -c16- /tmp/plotdata | grep -v -i "^[a-z]" > 2plotdata.txt
rm -f /tmp/plotdata
echo "Done, data file for gnuplot is 2plotdata.txt
(follow the steps in the LKP book, Ch 8, to plot the graph)."
ls -l 2plotdata.txt
