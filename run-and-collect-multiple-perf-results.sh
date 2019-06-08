#!/bin/bash

# sudo sysctl kernel.nmi_watchdog=0
# run using ./run-and-collect-multiple-perf-results.sh 2>/dev/null 1>/dev/null
trap 'exit 1' SIGINT

# TASK 1
TASK_1_TIMES=2

TASK_1_START_N=256
TASK_1_N_STEP=256
TASK_1_END_N=2944

# TASK_1_VERSIONS={0...3}
TASK_1_VERSIONS=`seq 0 3`

echo $TASK_1_VERSIONS
for n in `seq $TASK_1_START_N $TASK_1_N_STEP $TASK_1_END_N`
do
    for version in $TASK_1_VERSIONS
    do
        ./run-and-collect-perf-results.sh 1 0 $TASK_1_TIMES "task-1/matmult-n-${n}-v-${version}" ./matmult -n $n -v $version
    done
done

# terminate sudo session
sudo -k
