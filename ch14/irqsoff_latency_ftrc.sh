#!/bin/bash
# ch14/irqsoff_latency_ftrc.sh
#----------------------------------------------------------------------
# This program is part of the source code released for the book
#  "Learn Linux Kernel Development"
#  (c) Author: Kaiwan N Billimoria
#  Publisher:  Packt
#  GitHub repository:
#  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
# 
# For details, refer the book, Ch 14.
#----------------------------------------------------------------------
# Brief Description:
# This is a simple wrapper script; we use it to gauge the maximum IRQs-off
# latency on the system. It exploits the kernel's Ftrace infrastructure.
# For details, refer the book, Ch 14 - Handling hardware interrupts, section
# 'Finding the interrupts disabled worst-case latency with Ftrace'.
name=$(basename $(realpath $0))

reset_ftrc()
{
[ -x trace-cmd ] || return
echo "[+] resetting ftrace..."
trace-cmd reset
}

### 'main'

if [ $(id -u) -ne 0 ] ; then
  echo "${name}: must run as root."
  exit 1
fi

DEBUGFS_MNT=/sys/kernel/debug
[ ! -d ${DEBUGFS_MNT} ] && {
  echo "${name}: debugfs not present or mounted? aborting..."
  exit 1
}
FTRC=${DEBUGFS_MNT}/tracing
[ ! -d ${FTRC} ] && {
  echo "${name}: ${DEBUGFS_MNT}/tracing - ftrace - not present or mounted? aborting..."
  exit 1
}

grep -w -q "irqsoff" ${FTRC}/available_tracers || {
  echo "${name}: 'irqsoff' tracer not available, aborting..."
  echo "This implies you will have to configure your kernel (or a custom one),
with the CONFIG_IRQSOFF_TRACER kernel config option turned on."
  exit 1
}

reset_ftrc
echo "[+] 'irqsoff' tracer available, proceeding with latency test..."
echo 0 > ${FTRC}/tracing_on   # turn tracing off
echo 1 > ${FTRC}/options/latency-format   # turn latency format on
echo irqsoff > ${FTRC}/current_tracer
echo 1 > ${FTRC}/tracing_on   # turn tracing on

echo -n "${name}: irqsoff max latency : "
cat ${FTRC}/tracing_max_latency
cp ${FTRC}/trace /tmp/mytrc.txt
ls -lh /tmp/mytrc.txt
#cat /tmp/mytrc.txt
echo "${name}: examine the trace file to see where the max latency occurred"

reset_ftrc
exit 0
