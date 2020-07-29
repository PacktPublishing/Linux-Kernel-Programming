#!/bin/bash
# irqsoff_latency_ftrc.sh
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
