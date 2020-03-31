#!/bin/bash
# ASLR_check.sh
# ***************************************************************
# * This program is part of the source code released for the book
# *  "Learn Linux Kernel Development"
# *  (c) Author: Kaiwan N Billimoria
# *  Publisher:  Packt
# *  GitHub repository:
# *  https://github.com/PacktPublishing/Learn-Linux-Kernel-Development
# *
# * From: Ch 6 : Kernel and Memory Management Internals Essentials
# ****************************************************************
# * Brief Description:
# [K]ASLR = [Kernel] Address Space Layout Randomization
#
# * For details, please refer the book, Ch 6.
# ****************************************************************
name=$(basename $0)

test_ASLR_abit()
{
echo "
Quick test: doing
 egrep \"heap|stack\" /proc/self/maps
twice:
"

egrep "heap|stack" /proc/self/maps
echo
egrep "heap|stack" /proc/self/maps

echo "
With ASLR:
  enabled: the uva's (user virtual addresses) should differ in each run
 disabled: the uva's (user virtual addresses) should be the same in each run.
 "
}

# ASLR_set
# Parameters:
# $1 : integer; value to set ASLR to
ASLR_set()
{
echo "
[+] Setting (usermode) ASLR value to \"$1\" now..."
echo -n $1 > /proc/sys/kernel/randomize_va_space
echo -n "ASLR setting now is: "
cat /proc/sys/kernel/randomize_va_space

case "$1" in
 0) echo " : turn OFF ASLR" ;;
 1) echo " : turn ON ASLR only for stack, VDSO, shmem regions" ;;
 2) echo " : turn ON ASLR for stack, VDSO, shmem regions and data segments [OS default]" ;;
 *) echo "Invalid!" ; exit 2 ;;
esac
}

kernel_ASLR_check()
{
local KCONF KASLR_CONF

echo "
[+] Checking for kernel ASLR (KASLR) support now ...
(this kernel is ver $(uname -r), need >= 3.14)"

# KASLR: from 3.14 onwards
local mj=$(uname -r |awk -F"." '{print $1}')
local mn=$(uname -r |awk -F"." '{print $2}')
[ ${mj} -lt 3 ] && {
	echo " KASLR : 2.6 or earlier kernel, no KASLR support (very old kernel?)"
	exit 1
}
[ ${mj} -eq 3 -a ${mn} -lt 14 ] && {
	echo " KASLR : 3.14 or later kernel required for KASLR support."
	exit 1
}

# ok, we're on >= 3.14
grep -q -w "nokaslr" /proc/cmdline && {
  echo " Kernel ASLR (KASLR) turned OFF! (via kernel cmdline; unusual)"
} || {
  # Attempt to gain access to the kernel config; first via /proc/config.gz
  # and, if unavailable, via the /boot/config-<kver> file
  sudo modprobe configs 2>/dev/null
  if [ -f /proc/config.gz ] ; then
    gunzip -c /proc/config.gz > /tmp/kconfig
    KCONF=/tmp/kconfig
  elif [ -f /boot/config-$(uname -r) ] ; then
    KCONF=/boot/config-$(uname -r)
  else
    echo "${name}: FATAL: oops, cannot gain access to kernel config, aborting..."
    exit 1
  fi

  if [ ! -s ${KCONF} ]; then
    echo "${name}: FATAL: oops, invalid kernel config file (${KCONF})? Aborting..."
    exit 1
  fi

  KASLR_CONF=$(grep CONFIG_RANDOMIZE_BASE ${KCONF} |awk -F"=" '{print $2}')
  if [ "${KASLR_CONF}" = "y" ]; then
	  echo " Kernel ASLR (KASLR) is On [default]"
  else 
	grep -q -w "kaslr" /proc/cmdline && \
		echo " Kernel ASLR (KASLR) turned ON via cmdline" || \
			echo " Kernel ASLR (KASLR) is OFF!"
  fi
}
}

usermode_ASLR_check()
{
local UASLR=$(cat /proc/sys/kernel/randomize_va_space)

echo "[+] Checking for userspace ASLR support now ...
 (in /proc/sys/kernel/randomize_va_space)
 Current userspace ASLR setting = ${UASLR}"

case "${UASLR}" in
 0) echo " => userspace ASLR is turned OFF" ;;
 1) echo " => userspace ASLR ON for stack, VDSO, shmem regions" ;;
 2) echo " => userspace ASLR ON for stack, VDSO, shmem regions and data segments [OS default]" ;;
 *) echo " => invalid value? (shouldn't occur!)" ;;
esac
}

usage()
{
  local SEP="+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
  echo "${SEP}
Simple [Kernel] Address Space Layout Randomization / [K]ASLR checks:

Usage: ${name} [ASLR_value] ; where 'ASLR_value' is one of:
 0 = turn OFF ASLR
 1 = turn ON ASLR only for stack, VDSO, shmem regions
 2 = turn ON ASLR for stack, VDSO, shmem regions and data segments [OS default]

The 'ASLR_value' parameter, setting the ASLR value, is optional; in any case,
I shall run the checks... thanks and visit again!
${SEP}
"
}


##--------------- "main" here ---------------
[ $(id -u) -ne 0 ] && {
  echo "${name}: require root"
  exit 1
}
usage

[ ! -f /proc/sys/kernel/randomize_va_space ] && {
	echo "${name}: ASLR : no support (very old kernel (or, unlikely,"
	" /proc not mounted)?)"
	exit 1
}

usermode_ASLR_check
kernel_ASLR_check

if [ $# -eq 1 ] ; then
	echo "[+] Attempting to set ASLR value to \"$1\" now ..."
	if [ $1 -ne 0 -a $1 -ne 1 -a $1 -ne 2 ] ; then
		echo "${name}: invalid ASLR value (\"$1\"), aborting ..."
		usage
		exit 1
	else
		ASLR_set $1
	fi
fi
test_ASLR_abit

exit 0
