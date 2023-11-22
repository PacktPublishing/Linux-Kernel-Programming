#!/bin/bash
# ch13/3_lockdep/lock_stats_demo.sh
# ***************************************************************
# This program is part of the source code released for the book
#  "Linux Kernel Programming"
#  (c) Author: Kaiwan N Billimoria
#  Publisher:  Packt
#  GitHub repository:
#  https://github.com/PacktPublishing/Linux-Kernel-Programming
#****************************************************************
# Brief Description:
# Simple script to demo some kernel locking statistics.
# [This script was initially not in the repo; apologies!]
name=$(basename $0)

die()
{
echo >&2 "FATAL: $@"
exit 1
}

clear_lock_stats() {
  echo 0 > /proc/lock_stat
}
enable_lock_stats() {
  echo 1 > /proc/sys/kernel/lock_stat
}
disable_lock_stats() {
  echo 0 > /proc/sys/kernel/lock_stat
}
view_lock_stats() {
  cat /proc/lock_stat
}


#--- 'main'
[[ -f /boot/config-$(uname -r) ]] && { # at least on x86[_64]
   grep -w "CONFIG_LOCK_STAT=y" /boot/config-$(uname -r) >/dev/null 2>&1
   [[ $? -ne 0 ]] && die "${name}: you're running this on a kernel without lock stats (CONFIG_LOCK_STAT) enabled"
}

[[ $(id -u) -ne 0 ]] && die "Needs root."

disable_lock_stats

#--- Test case --------------------------------------------------------
# We want to see some kernel locks and their stats.
# So run a simple command and capture the stats.
# Enable lock stats only now!
clear_lock_stats
enable_lock_stats
cat /proc/self/cmdline
#----------------------------------------------------------------------

disable_lock_stats
REPFILE=lockstats.txt
view_lock_stats > ${REPFILE}
#view_lock_stats |tee ${REPFILE}
echo "${name}: done, see the kernel locking stats in ${REPFILE}"
exit 0
