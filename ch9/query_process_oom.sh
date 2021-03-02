#!/bin/bash
# query_process_oom.sh
# ***************************************************************
# This program is part of the source code released for the book
#  "Linux Kernel Programming"
#  (c) Author: Kaiwan N Billimoria
#  Publisher:  Packt
#  GitHub repository:
#  https://github.com/PacktPublishing/Linux-Kernel-Programming
# ****************************************************************
# Brief Description:
#
# Query the OOM score of all processes currently alive on the system.
# Just a simple wrapper around choom(1).
# Tip: One can always arrange to sort the output by OOM score; f.e.:
#  /query_task_oom.sh |sed '1d' |sort -k3n
# Details: refer to the LKP book, Ch 9
i=1
printf "  PID                      Name         OOM Score\n"

IFS=$'\n'
for rec in $(ps -A)
do
  [ $i -eq 1 ] && {  # skip ps's header
    let i=i+1
    continue
  }

  # 'grok' & extract it, and ...
  pid=$(echo "${rec}" |awk '{print $1}')
  comm=$(echo "${rec}" |awk '{print $4}')
  rec2=$(choom -p ${pid} 2>/dev/null)
  oom_score=$(echo "${rec2}" |head -n1|cut -d":" -f2)

  # ... print it!
  [ ${pid} -ne 0 ] && {
     printf "%9d  %28s" ${pid} ${comm}
     printf "   %6d" ${oom_score}
  }
  printf "\n"
  let i=i+1
done
exit 0
