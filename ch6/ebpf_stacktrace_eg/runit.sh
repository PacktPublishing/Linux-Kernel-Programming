#!/bin/bash
# ch6/ebpf_stacktrace_eg/runit.sh
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
# * Script to demo using the stackcount-bpfcc BCC tool to trace both kernel
# * and user-mode stacks of our Hello, world process for the write(s)
# *
# * For details, please refer the book, Ch 6.
# ****************************************************************
[ ! -f ./helloworld_dbg ] && {
  echo "Pl build the helloworld_dbg program first... (with 'make')"
  exit 1
}

pkill helloworld_dbg 2>/dev/null
./helloworld_dbg >/dev/null &
PID=$(pgrep helloworld_dbg)
#PID=$(ps -e|grep "helloworld_dbg" |tail -n1|awk '{print $1}')
[ -z "${PID}" ] && {
  echo "Oops, could not get PID of the helloworld_dbg process, aborting..."
  exit 1
}

# Ubuntu specific name for BCC tool(s), pl adjust as required for other distros
PRG=stackcount-bpfcc

which ${PRG} >/dev/null
[ $? -ne 0 ] && {
  echo "Oops, ${PRG} not installed? aborting..."
  exit 1
}

echo "sudo ${PRG} -p ${PID} -r ".*sys_write.*" -v -d"
sudo ${PRG} -p ${PID} -r ".*sys_write.*" -v -d
exit 0
