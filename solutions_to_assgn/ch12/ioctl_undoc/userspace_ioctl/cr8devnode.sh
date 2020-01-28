#!/bin/bash
# cr8devnode.sh
# Simple utility script to create the device node for the driver
# (Binds on the basis of the OURMODNAME macro; so simply update it...
# also, we just set the minor # to 0)
name=$(basename $0)
OURMODNAME="ioctl_undoc_kdrv"
DEVNM=/dev/ioctl_undoc

MAJOR=$(grep "${OURMODNAME}" /proc/devices |awk '{print $1}')
[ -z "${MAJOR}" ] && {
  echo "${name}: failed to retreive the major #, aborting ...
  (is the ${OURMODNAME} device driver loaded ?)"
  exit 1
}
echo "major number is ${MAJOR}"
sudo rm -f "${DEVNM}"     # rm any stale instance
sudo mknod "${DEVNM}" c ${MAJOR} 0
ls -l "${DEVNM}"
exit 0
