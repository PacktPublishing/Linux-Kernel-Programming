#!/bin/bash
# ASLR_check.sh
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
# [K]ASLR = [Kernel] Address Space Layout Randomization
#
# * For details, please refer the book, Ch 6.
# ****************************************************************
name=$(basename "$0")
PFX=$(dirname "$(which "$0")")    # dir in which this script resides
source ${PFX}/color.sh || {
 echo "${name}: fatal: could not source ${PFX}/color.sh , aborting..."
 exit 1
}
SEP="+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"

test_ASLR_abit()
{
tput bold; fg_purple
echo "${SEP}
ASLR quick test:"
color_reset

echo "Doing
 grep -E \"heap|stack\" /proc/self/maps
twice:
"

fg_blue
grep -E "heap|stack" /proc/self/maps
echo
fg_cyan
grep -E "heap|stack" /proc/self/maps
color_reset

echo "
With ASLR:
  enabled: the uva's (user virtual addresses) should differ in each run
 disabled: the uva's (user virtual addresses) should be the same in each run.
 "
}

# disp_aslr_by_value
# Parameters:
#  $1 : integer; ASLR value to interpret
disp_aslr_by_value()
{
case "$1" in
 0) tput bold ; fg_red
	echo " => (usermode) ASLR is curently OFF"
	;;
 1) tput bold ; fg_yellow; echo " => (usermode) ASLR ON: mmap(2)-based allocations, stack, vDSO page,"
    echo " shlib, shmem locations are randomized on startup"
	;;
 2) tput bold ; fg_green
    echo " => (usermode) ASLR ON: mmap(2)-based allocations, stack, vDSO page,"
    echo " shlib, shmem locations and heap are randomized on startup"
	;;
 *) tput bold ; fg_red ; echo " => invalid value? (shouldn't occur!)" ;;
esac
color_reset
}

# ASLR_set
# Parameters:
#  $1 : integer; value to set ASLR to
ASLR_set()
{
tput bold ; fg_purple
echo "${SEP}
[+] Setting (usermode) ASLR value to \"$1\" now..."
color_reset
echo -n "$1" > /proc/sys/kernel/randomize_va_space
echo -n "ASLR setting now is: "
cat /proc/sys/kernel/randomize_va_space
disp_aslr_by_value "$(cat /proc/sys/kernel/randomize_va_space)"
}

kernel_ASLR_check()
{
local KCONF KASLR_CONF

tput bold ; fg_purple
echo "${SEP}
[+] Checking for kernel ASLR (KASLR) support now ..."
color_reset
echo "(need >= 3.14, this kernel is ver $(uname -r))"

# KASLR: from 3.14 onwards
local mj=$(uname -r |awk -F"." '{print $1}')
local mn=$(uname -r |awk -F"." '{print $2}')
[ "${mj}" -lt 3 ] && {
	tput bold ; fg_red
	echo " KASLR : 2.6 or earlier kernel, no KASLR support (very old kernel?)"
	color_reset
	exit 1
}
[ "${mj}" -eq 3 -a "${mn}" -lt 14 ] && {
	tput bold ; fg_red ; echo " KASLR : 3.14 or later kernel required for KASLR support."
	color_reset
	exit 1
}

# ok, we're on >= 3.14
grep -q -w "nokaslr" /proc/cmdline && {
  tput bold ; fg_red ; echo " Kernel ASLR (KASLR) turned OFF (via kernel cmdline)"
  color_reset
  return
}

# Attempt to gain access to the kernel config; first via /proc/config.gz
# and, if unavailable, via the /boot/config-<kver> file
sudo modprobe configs 2>/dev/null
if [ -f /proc/config.gz ] ; then
    gunzip -c /proc/config.gz > /tmp/kconfig
    KCONF=/tmp/kconfig
elif [ -f /boot/config-"$(uname -r)" ] ; then
    KCONF=/boot/config-$(uname -r)
else
	tput bold ; fg_red
    echo "${name}: FATAL: oops, cannot gain access to kernel config, aborting..."
	color_reset
    exit 1
fi

if [ ! -s "${KCONF}" ]; then
	tput bold ; fg_red
    echo "${name}: FATAL: oops, invalid kernel config file (${KCONF})? Aborting..."
	color_reset
    exit 1
fi

KASLR_CONF=$(grep CONFIG_RANDOMIZE_BASE "${KCONF}" |awk -F"=" '{print $2}')
if [ "${KASLR_CONF}" = "y" ]; then
	tput bold ; fg_green
	echo " Kernel ASLR (KASLR) is On [default]"
	color_reset
else 
	grep -q -w "kaslr" /proc/cmdline && \
		echo " Kernel ASLR (KASLR) turned ON via cmdline" || {
			tput bold ; fg_red
			echo " Kernel ASLR (KASLR) is OFF!"
			color_reset
		}
fi
}

usermode_ASLR_check()
{
local UASLR=$(cat /proc/sys/kernel/randomize_va_space)

tput bold ; fg_purple
echo "${SEP}
[+] Checking for (usermode) ASLR support now ..."
color_reset
echo " (in /proc/sys/kernel/randomize_va_space)
 Current (usermode) ASLR setting = ${UASLR}"

grep -q -w "norandmaps" /proc/cmdline && {
	tput bold ; fg_red
	echo " (Usermode) ASLR turned OFF via kernel cmdline"
	color_reset
	return
}
disp_aslr_by_value ${UASLR}
}

usage()
{
echo "${SEP}"
tput bold
echo "Simple [Kernel] Address Space Layout Randomization / [K]ASLR checks:"
color_reset
echo "Usage: ${name} [ASLR_value] ; where 'ASLR_value' is one of:
 0 = turn OFF ASLR
 1 = turn ON ASLR only for stack, VDSO, shmem regions
 2 = turn ON ASLR for stack, VDSO, shmem regions and data segments [OS default]

The 'ASLR_value' parameter, setting the ASLR value, is optional; in any case,
I shall run the checks... thanks and visit again!"
}


##--------------- "main" here ---------------
[ $(id -u) -ne 0 ] && {
  echo "${name}: require root"
  exit 1
}
usage

[ ! -f /proc/sys/kernel/randomize_va_space ] && {
	tput bold ; fg_red
	echo "${name}: ASLR : no support (very old kernel (or, unlikely,"
	" /proc not mounted)?)"
	exit 1
}

usermode_ASLR_check
kernel_ASLR_check

if [ $# -eq 1 ] ; then
	if [ $1 -ne 0 -a $1 -ne 1 -a $1 -ne 2 ] ; then
		fg red ; echo
		echo "${name}: set ASLR: invalid value (\"$1\"), aborting ..."
		color_reset
		exit 1
	else
		ASLR_set "$1"
	fi
fi
test_ASLR_abit

exit 0
