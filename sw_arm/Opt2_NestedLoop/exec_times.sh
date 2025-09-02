#!/bin/sh

N=32
M=32
echo " ###################### N = $N, M = $M ###################### " >> ops.out
echo " ###################### N = $N, M = $M ###################### " >> bytes.out
for i in {0..11}
do
    ./lsal_sw_arm $N $M >> tmp_times.txt
done
echo " ###################### N = $N, M = $M ###################### " >> exec_times.txt
./calc_mean 0 >> exec_times.txt
./calc_mean 1 >> mean_times.dat
rm -rf tmp_times.txt

N=32
M=65536
echo " ###################### N = $N, M = $M ###################### " >> ops.out
echo " ###################### N = $N, M = $M ###################### " >> bytes.out
for i in {0..11}
do
    ./lsal_sw_arm $N $M >> tmp_times.txt
done
echo " ###################### N = $N, M = $M ###################### " >> exec_times.txt
./calc_mean 0 >> exec_times.txt
./calc_mean 1 >> mean_times.dat
rm -rf tmp_times.txt

N=256
M=65536
echo " ###################### N = $N, M = $M ###################### " >> ops.out
echo " ###################### N = $N, M = $M ###################### " >> bytes.out
for i in {0..11}
do
    ./lsal_sw_arm $N $M >> tmp_times.txt
done
echo " ###################### N = $N, M = $M ###################### " >> exec_times.txt
./calc_mean 0 >> exec_times.txt
./calc_mean 1 >> mean_times.dat
rm -rf tmp_times.txt

N=256
M=300000
echo " ###################### N = $N, M = $M ###################### " >> ops.out
echo " ###################### N = $N, M = $M ###################### " >> bytes.out
for i in {0..11}
do
    ./lsal_sw_arm $N $M >> tmp_times.txt
done
echo " ###################### N = $N, M = $M ###################### " >> exec_times.txt
./calc_mean 0 >> exec_times.txt
./calc_mean 1 >> mean_times.dat
rm -rf tmp_times.txt