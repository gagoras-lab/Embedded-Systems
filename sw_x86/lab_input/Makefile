COMPILER = g++
FLAGS = -Wall -g -O3 -mavx2 -fopenmp
CFILE = lsal_padding_opt3_copy.cpp
TARGET = lsal_sw_x86
N = 8 
M = 9
DEBUG = #-DDEBUG

lsal_sw_x86: 
	$(COMPILER) $(FLAGS) $(DEBUG) $(CFILE) -o $(TARGET)

run: 
	./$(TARGET) $(N) $(M)

clean: 
	rm -rf $(TARGET)