#!/bin/sh
# ch11/cgroups_v2_cpu_eg/simp.sh
# ***************************************************************
# This program is part of the source code released for the book
#  "Linux Kernel Programming"
#  (c) Author: Kaiwan N Billimoria
#  Publisher:  Packt
#  GitHub repository:
#  https://github.com/PacktPublishing/Linux-Kernel-Programming
# ****************************************************************
# Brief Description:
# Part of the cgroups v2 CPU controller test script.
# This simple script hammers the CPU(s), emitting integer values...
# The value to start with is passed as a parameter to it; it's used to
# get an approximation of the CPU loading.
# Do NOT invoke this script directly; it's meant to be invoked from the
# ch11/cgroups_v2_cpu_eg/cgv2_cpu_ctrl.sh bash script.
#
# For details, pl refer to the book Ch 11.

# 1st param is starting #
[ $# -ne 1 ] && {
 echo "Usage: $0 <starting#>"
 exit 1
}
max=9999
[ $1 -ge $max ] && {
 echo "Param $1 >= max ($max)"
 exit 1
}

delay_loop()
{
SEQ_MAX1=10
SEQ_MAX2=200
for x in $(seq 1 ${SEQ_MAX1}); do
  for y in $(seq 1 ${SEQ_MAX2}); do
    z=$(((y+10000)*x/y))    
  done                      
done                        
}                    
                     
i=$1                 
while [ $i -le $max ]
do                   
  echo -n "$i "
  i=$((i+1))   
  delay_loop
done
