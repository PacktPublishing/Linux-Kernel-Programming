#!/bin/bash
# plotter_prep.sh
# Book: Linux Kernel Development Cookbook, Kaiwan N Billimoria, Packt.
# Part of the ch5/slab4_actualsz_wstg_plot code.
#
# Script to help prepare the data file from kernel log.
# We assume that:
# a) sudo dmesg -C   ; was done prior to the kernel module being inserted
#    (clearing any the stale stuff)
# b) the cut(1) below gets rid of the dmesg timestamp; we assume it's
#    (timestamp) there
# c) you will comment out or delete any extraneous lines in the final o/p
# file 2plotdata.txt after this :-)
# (To save the trouble, we've (also) kept the 2plotdata.txt file in the repo).
dmesg > plotdata.txt
cut -c16- plotdata.txt > 2plotdata.txt
