#!/bin/bash
# countem.sh
# ***************************************************************
# * This program is part of the source code released for the book
# *  "Learn Linux Kernel Development"
# *  (c) Author: Kaiwan N Billimoria
# *  Publisher:  Packt
# *  GitHub repository:
# *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
# *
# * From: Ch 6 : Kernel and Memory Management Internals Essentials
# ****************************************************************
# * Brief Description:
# *
# * For details, please refer the book, Ch 6.
# ****************************************************************
echo "System release info:"
lsb_release -a 2>/dev/null

total_prcs=$(ps -A|wc -l)
printf "\nTotal # of processes alive              = %9d\n" ${total_prcs}
total_thrds=$(ps -LA|wc -l)
printf "Total # of threads alive                = %9d\n" ${total_thrds}
total_kthrds=$(ps aux|awk '{print $11}'|grep "^\["|wc -l)
printf "Total # of kernel threads alive         = %9d\n" ${total_kthrds}
printf "Thus, total # of usermode threads alive = %9d\n" $((${total_thrds}-${total_kthrds}))

exit 0
