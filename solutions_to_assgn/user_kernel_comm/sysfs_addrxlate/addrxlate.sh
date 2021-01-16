#!/bin/bash
# addrxlate.sh
# **********************************************************************
# This program is part of the source code released for the book
#  "Learn Linux Kernel Development"
#  (c) Author: Kaiwan N Billimoria
#  Publisher:  Packt
#  GitHub repository:
#  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
# **********************************************************************
# Brief Description:
# Simple script wrapper over the sysfs_addrxlate LKM.
# For details, refer to the book Ch 6.
name=$(basename $0)
MOD=sysfs_addrxlate
SYSFS_FILE=/sys/devices/platform/llkd_sysfs_addrxlate/addrxlate_kva2pa

usage()
{
  echo "Usage: ${name} -k|-p address-to-translate (in hex 0x...)

 Option: -k
  Translate the provided kernel virtual address (kva) (2nd parameter) to physical addr

  NOTE! The kva passed as parameter MUST be a valid kernel logical addr;
  it MUST lie within the kernel 'lowmem' segment, i.e., between PAGE_OFFSET
  and high_memory
  Eg. on a 32-bit system with a 3:1 VM split:
    ${name} -k 0xc000a000      <-- specify addr in hex pl

 Option: -p
  Translate the provided physical address (2nd parameter) to it's corresponding
  kernel logical address (kva);
  MUST be a valid physical address
  Eg. on a 32-bit system with a 3:1 VM split:
    ${name} -p 0xa000      <-- specify addr in hex pl"
}

setup()
{
lsmod |grep -q ${MOD} || {
  echo "${name}: kernel module '${MOD}' not loaded; will attempt loading it now..."
  [ -f ${MOD}.ko ] || {
    echo "${name}: kernel module not built? aborting"
    exit 1
  }
  sudo rmmod ${MOD} 2>/dev/null
  sudo insmod ./${MOD}.ko || {
    echo "${name}: kernel module '${MOD}.ko' load failed, aborting..."
    exit 1
  }
  echo "[+] kernel module loaded"
}
[ -e ${SYSFS_FILE} ] || {
  echo "${name}: sysfs file \"${SYSFS_FILE}\" missing? aborting...(check kernel log)"
  exit 1
}
}

# kva2pa()
# Parameters:
#  $1 : the kva to translate
kva2pa()
{
echo "[+] Translating kva --> pa
 writing user-supplied kva '${1}' to our driver; see o/p (xlated phy addr):
(Tip: if it doesn't work, see the kernel log)
"

# Write the kva to transalte to pa to appropriate sysfs file (as root)
echo $1 > /sys/devices/platform/llkd_sysfs_addrxlate/addrxlate_kva2pa
#sudo sh -c "echo $1 > /sys/devices/platform/llkd_sysfs_addrxlate/addrxlate_kva2pa"
# Read the result and display
cat /sys/devices/platform/llkd_sysfs_addrxlate/addrxlate_kva2pa ; echo
}

# pa2kva()
# Parameters:
#  $1 : the pa to translate
pa2kva()
{
echo "[+] Translating pa --> kva
 writing user-supplied pa '${1}' to our driver; see o/p (xlated phy addr):
(Tip: if it doesn't work, see the kernel log)
"

# Write the pa to transalte to kva to appropriate sysfs file (as root)
echo $1 > /sys/devices/platform/llkd_sysfs_addrxlate/addrxlate_pa2kva
#sudo sh -c "echo $1 > /sys/devices/platform/llkd_sysfs_addrxlate/addrxlate_pa2kva"
# Read the result and display
cat /sys/devices/platform/llkd_sysfs_addrxlate/addrxlate_pa2kva ; echo
}

### 'main' here

[ $(id -u) -ne 0 ] && {
  echo "${name}: must run as root."
  exit 1
}

# argument processing
[ $# -ne 2 ] && {
  usage
  exit 1
}
case "$1" in
  -k|-K) 
	setup
	kva2pa $2
	;;
  -p|-P) 
	setup
	pa2kva $2
	;;
  *) usage ; exit 1
esac

exit 0
