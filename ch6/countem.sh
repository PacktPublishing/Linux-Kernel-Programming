#!/bin/bash
# countem.sh
# ***************************************************************
# * This program is part of the source code released for the book
# *  "Linux Kernel Programming"
# *  (c) Author: Kaiwan N Billimoria
# *  Publisher:  Packt
# *  GitHub repository:
# *  https://github.com/PacktPublishing/Linux-Kernel-Programming
# *
# * From: Ch 6 : Kernel and Memory Management Internals Essentials
# ****************************************************************
# * Brief Description:
# *
# * For details, please refer the book, Ch 6.
# ****************************************************************
echo "System release info:"
which lsb_release >/dev/null
if [ $? -eq 0 ] ; then
  lsb_release -a 2>/dev/null
else
  [ -f /etc/issue ] && cat /etc/issue
  [ -f /etc/os-release ] && cat /etc/os-release
fi

total_prcs=$(ps -A|wc -l)
printf "\nTotal # of processes alive              = %9d\n" ${total_prcs}

# ps -LA shows all threads
total_thrds=$(ps -LA|wc -l)
printf "Total # of threads alive                = %9d\n" ${total_thrds}

# ps aux shows all kernel threads names (col 11) in square brackets; count 'em
total_kthrds=$(ps aux|awk '{print $11}'|grep "^\["|wc -l)

printf "Total # of kernel threads alive         = %9d\n" ${total_kthrds}
printf "Thus, total # of usermode threads alive = %9d\n" $((${total_thrds}-${total_kthrds}))

exit 0
