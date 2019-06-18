#!/bin/bash
# Query the scheduling attributes (policy and RT (static) priority) of 
# all processes currently alive on the system.
# Just a simple wrapper around chrt.
# 
# Tip: Pipe this o/p to grep for FIFO / RR tasks..
# Also note that a multithreaded process shows up as several same PIDs
#  (resolve these using ps -eLf - to see actual PIDs of threads).

i=1
printf "  PID       TID            Name                 Sched Policy Prio\n"
IFS=$'\n'
for rec in $(ps -LA)
#for rec in $(ps -LA -To pid,comm)
do
  [ $i -eq 1 ] && {  # skip ps's hdr
    let i=i+1
    continue
  }

  # 'grok' & extract it, and ...
  pid=$(echo "${rec}" |awk '{print $1}')
  tid=$(echo "${rec}" |awk '{print $2}')
  comm=$(echo "${rec}" |awk '{print $5}')

  rec2=$(chrt -p ${pid} 2>/dev/null)
  rec2_line1=$(echo "${rec2}" |head -n1)
  rec2_line2=$(echo "${rec2}" |tail -n1)
  policy=$(echo ${rec2_line1} |awk -F: '{print $2}')
  prio=$(echo ${rec2_line2} |awk -F: '{print $2}')

  # ... print it!
  [ "${pid}" != "0" ] && {
     printf "%6d  %6d  %28s   %12s   %2d" ${pid} ${tid} ${comm} ${policy} ${prio}
     if [ "${policy}" = " SCHED_RR" -o "${policy}" = " SCHED_FIFO" ] ; then
        if [ ${prio} -eq 99 ] ; then
	   printf "     ***"
	fi
     fi
  }
  printf "\n"
  let i=i+1
done
exit 0
