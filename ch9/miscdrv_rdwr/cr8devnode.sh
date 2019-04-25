#!/bin/bash
# cr8devnode.sh
# Simple utility script to create the device node for the miscdrv_rdwr 'misc'
# class device driver
name=$(basename $0)

MAJOR=10   # misc class is always major # 10
unalias dmesg 2>/dev/null
MINOR=$(dmesg |grep "miscdrv_rdwr\:minor\=" |cut -d"=" -f2)
[ -z "${MINOR}" ] && {
  echo "${name}: failed to retreive the minor #, aborting ..."
  exit 1
}
echo "minor number is ${MINOR}"

sudo mknod /dev/miscdrv c ${MAJOR} ${MINOR}
ls -l /dev/miscdrv
exit 0
