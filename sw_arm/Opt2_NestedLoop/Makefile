#COMPILER = /opt/Xilinx/Vitis/2020.2/gnu/aarch32/lin/gcc-arm-linux-gnueabi/bin/arm-linux-gnueabihf-g++
COMPILER = gcc
FLAGS = -Wall -g -O3 -fmessage-length=0
CFILE = ./lsal_nested.cpp
TARGET = ./lsal_sw_arm
N = 8 
M = 9
DEBUG = #-DDEBUG

all: lsal_sw_arm calc_mean

lsal_sw_arm: 
	$(COMPILER) -o $(TARGET) $(CFILE) $(FLAGS) $(DEBUG) 

calc_mean:
	$(COMPILER) -o ./calc_mean ./calc_mean.c $(FLAGS)

run: 
	./$(TARGET) $(N) $(M)

clean: 
	rm -rf $(TARGET) calc_mean
