#!/bin/sh

for i in {0..9}
do
    ./lsal_sw_x86 256 300000 >> exec_times_new.txt
done