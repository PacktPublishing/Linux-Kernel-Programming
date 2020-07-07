#!/bin/bash

[ ! -f ./helloworld_dbg ] && {
  echo "Pl build the helloworld_dbg program first... (with 'make')"
  exit 1
}

pkill helloworld_dbg 2>/dev/null
./helloworld_dbg >/dev/null &
PID=$(ps -e|grep "helloworld_dbg" |tail -n1|awk '{print $1}')
[ -z "${PID}" ] && {
  echo "Oops, could not get PID of helloworld_dbg, aborting..."
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
