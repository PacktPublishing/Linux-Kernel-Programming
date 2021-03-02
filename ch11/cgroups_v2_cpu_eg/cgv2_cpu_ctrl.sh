#!/bin/bash
# ch11/cgroups_v2_cpu_eg/cgv2_cpu_ctrl.sh
# ***************************************************************
# This program is part of the source code released for the book
#  "Linux Kernel Programming"
#  (c) Author: Kaiwan N Billimoria
#  Publisher:  Packt
#  GitHub repository:
#  https://github.com/PacktPublishing/Linux-Kernel-Programming
# ****************************************************************
# Brief Description:
# A quick test case for cgroups v2 CPU controller.
#
# For details, pl refer to the book Ch 11.
#                                                                      
# Additional Ref:
# https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html#cpu
# https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/8/html/managing_monitoring_and_updating_the_kernel/setting-limits-for-applications_managing-monitoring-and-updating-the-kernel#setting-cpu-limits-to-applications-using-cgroups-v2_using-control-groups-through-a-virtual-file-system
name=$(basename $0)
OUT1=1stjob.txt
OUT2=2ndjob.txt

TDIR=test_group

# cleanup
remove_subgroup()
{
echo "-cpu" > ${CGV2_MNT}/cgroup.subtree_control 2>/dev/null
[ -d ${CGV2_MNT}/${TDIR} ] && {
  echo "[+] Removing our cpu sub-group controller"
  echo "" > ${CGV2_MNT}/${TDIR}/cgroup.procs 2>/dev/null
  rmdir ${CGV2_MNT}/${TDIR}
}
} # end remove_subgroup()

cpu_resctrl_try()
{
echo "[+] Launch processes j1 and j2 (slinks to ${TD}/simp.sh) now ..."                                   
#--- Run a job j1                                                      
rm -f j1                                                               
ln -sf ${TD}/simp.sh j1                                                
./j1 1 >${OUT1} &                                                      
j1pid=$(ps -A|grep j1|head -n1|awk '{print $1}')                       
                                                                       
#--- Run a job j2
rm -f j2
ln -sf ${TD}/simp.sh j2
./j2 900 >${OUT2} &
j2pid=$(ps -A|grep j2|head -n1|awk '{print $1}')

echo "[+] Insert processes j1 and j2 into our new CPU ctrl sub-group"                                   
#--- Put j1 there
echo "${j1pid}" > ${CGV2_MNT}/${TDIR}/cgroup.procs
#--- Put j2 there
echo "${j2pid}" > ${CGV2_MNT}/${TDIR}/cgroup.procs
sleep 1

# Verify
echo "Verifying their presence..."
#cat /proc/${j1pid}/cgroup
grep "^0::/${TDIR}" /proc/${j1pid}/cgroup || echo "Warning! Job j1 not in our new cgroup v2 ${TDIR}" \
 && echo "Job j1 is in our new cgroup v2 ${TDIR}"
#cat /proc/${j2pid}/cgroup
grep "^0::/${TDIR}" /proc/${j2pid}/cgroup || echo "Warning! Job j2 not in our new cgroup v2 ${TDIR}" \
 && echo "Job j2 is in our new cgroup v2 ${TDIR}"
} # end cpu_resctrl_try()
                                                                       
setup_cgv2_cpu()
{
echo "[+] Adding a 'cpu' controller to the cgroups v2 hierarchy"                                   

echo "+cpu" > ${CGV2_MNT}/cgroup.subtree_control || {
  echo "Adding cpu controller failed, aborting.
Note:
a) Pl verify that you're exclusively running cgroups v2 (except for the systemd cgroup)
This is usually achieved by passing the kernel parameter
\"cgroup_no_v1=all\" at boot.
b) the presence of any RT process in this group will cause the 'cpu' controller addition to fail" ; exit 1
}

echo "[+] Create a sub-group under it (here: ${CGV2_MNT}/${TDIR})"
if [ ! -d ${CGV2_MNT}/${TDIR} ]; then
 mkdir ${CGV2_MNT}/${TDIR} || {
  echo "Warning! creating sub-dir ${CGV2_MNT}/${TDIR} failed..."
  exit 1
 }
fi
#ls -l ${CGV2_MNT}/${TDIR}

# cpu.max
#  A read-write two value file which exists on non-root cgroups. The default is
#  “max 100000”. The maximum bandwidth limit. It’s in the following format: 
#  $MAX $PERIOD
#  which indicates that the group may consume upto $MAX in each $PERIOD duration.
#  “max” for $MAX indicates no limit. If only one number is written, $MAX is
#  (the one that's) updated.
#
# In effect, all processes collectively in the sub-control group will be allowed
# to run for $MAX out of a period of $PERIOD; with MAX=200,000 and PERIOD=1,000,000
# we're effectively allowing all processes there to run for 0.2s out of a period of
# 1 second, i.e., utilizing 20% CPU bandwidth!
# The unit of $MAX and $PERIOD is microseconds.
local pct_cpu=$(bc <<< "scale=3; (${MAX}/${PERIOD})*100.0")
echo "
***
Now allowing ${MAX} out of a period of ${PERIOD} by all processes (j1,j2) in this
sub-control group, i.e., ${pct_cpu}% !
***
"
echo "${MAX} ${PERIOD}" > ${CGV2_MNT}/${TDIR}/cpu.max || {
  echo "Error! updating cpu.max for our sub-control group failed"
  exit 1
}
} # end setup_cgv2_cpu()

                                                                       
### "main" here                                                        
                                                                       
[ $(id -u) -ne 0 ] && {                                                
   echo "$0: need root."                                          
   exit 1                                                         
}
which bc >/dev/null || {
  echo "${name}: the 'bc' utility is  missing; pl install and retry"
  exit 1
}

MIN_BANDWIDTH_US=1000
PERIOD=1000000

[ $# -ne 1 ] && {
  echo "Usage: ${name} max-to-utilize(us)
 This value (microseconds) is the max amount of time the processes in the sub-control
 group we create will be allowed to utilize the CPU; it's relative to the period,
 which is the value ${PERIOD};
 So, f.e., passing the value 300,000 (out of 1,000,000) implies a max CPU utiltization
 of 0.3 seconds out of 1 second (i.e., 30% utilization).
 The valid range for the \$MAX value is [${MIN_BANDWIDTH_US}-${PERIOD}]."
 exit 1
}
MAX=$1
if [ ${MAX} -lt ${MIN_BANDWIDTH_US} -o ${MAX} -gt ${PERIOD} ] ; then
  echo "Your value for MAX (${MAX}) is invalid; must be in the range [${MIN_BANDWIDTH_US}-${PERIOD}]"
  exit 1
fi

echo "[+] Checking for cgroup v2 kernel support"
# Check whether booted w/ cgroup_no_v1=all parameter!
grep -q "cgroup_no_v1=all" /proc/cmdline || echo "Warning! unsure of cgroup v2 support, as kernel parameter \"cgroup_no_v1=all\"
hasn't been passed...
"

[ -f /proc/config.gz ] && {
  zcat /proc/config.gz |grep -q -i cgroup || {
    echo "$0: Cgroup support not builtin to kernel?? Aborting..."
    exit 1
  }
}

mount |grep -q cgroup2 || {
  echo "No cgroup2 filesystem mounted? Pl mount one first; aborting..."
  exit 1
}
export CGV2_MNT=$(mount |grep cgroup2 |awk '{print $3}')
[ -z "${CGV2_MNT}" ] && {
  echo "cgroup2 filesystem not acquired, aborting..."
  exit 1
}

TD=$(pwd)

remove_subgroup
setup_cgv2_cpu
cpu_resctrl_try

TIME_ALLOW=5  # seconds
echo "
............... sleep for ${TIME_ALLOW} s ................
"
sleep ${TIME_ALLOW}

# get rid of the sub-control group (it might be there a bit before completely disappearing)
echo "[+] killing processes j1, j2 ..."
kill -9 ${j1pid} ${j2pid} >/dev/null 2>&1
sleep 1

echo "cat ${OUT1}"
cat ${OUT1}
echo "
cat ${OUT2}"
cat ${OUT2} ; echo

remove_subgroup
exit 0
