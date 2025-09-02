#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "lsal.h"

// static long int cnt_ops=0;
// static long int cnt_bytes=0;

/**********************************************************************************************
 * LSAL kernel code to be implemented in Hardware
 * Inputs:
 *          string1 is the query[N]
 *          string2 is the database[M]
 *          input sizes N, M
 * Outputs:
 *           max_index is the location of the highest similiarity score 
 *           similarity and direction matrices. Note that these two matrices are initialized with zeros.
 **********************************************************************************************/
extern "C" {
void compute_matrices(
	char *string1, char *string2,
	int *max_index, int *similarity_matrix, short *direction_matrix, int N, int M)
{
#pragma HLS INTERFACE m_axi port=string1 offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=string1 bundle=control
#pragma HLS INTERFACE m_axi port=string2 offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=string2 bundle=control
#pragma HLS INTERFACE m_axi port=max_index offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=max_index bundle=control
#pragma HLS INTERFACE m_axi port=similarity_matrix offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=similarity_matrix bundle=control
#pragma HLS INTERFACE m_axi port=direction_matrix offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=direction_matrix bundle=control
#pragma HLS INTERFACE s_axilite port=N bundle=control
#pragma HLS INTERFACE s_axilite port=M bundle=control
#pragma HLS INTERFACE s_axilite port=return bundle=control
    int index = N+1;
    int i = 0;
    int j = 0;
    // Following values are used for the N, W, and NW values wrt. similarity_matrix[i]
    int north = 0;
	int west = 0;
	int northwest = 0;
	int max_value = 0;
	int match = 0;
	int val = 0;
	int test_val = 0;
	int dir = 0;

	//Here the real computation starts. Place your code whenever is required.
	// Scan the N*M array row-wise starting from the second row.

	for (i = 1; i < M; i++) {
#pragma HLS PIPELINE off
#pragma HLS loop_tripcount min=16 max=256
		for (j = 1; j < N; j++) {
#pragma HLS PIPELINE off
#pragma HLS loop_tripcount min=32 max=65536

			west = similarity_matrix[index - 1];
			north = similarity_matrix[index - N];
			northwest = similarity_matrix[index - N - 1];

			match = (string1[j - 1] == string2[i - 1]) ? MATCH : MISS_MATCH;
			test_val = northwest + match;
			val = (north >= west) ? north + GAP_d : west + GAP_i;

			if (test_val >= val) {
				val = test_val;
				dir = NORTH_WEST;
			}
			else if (val == north - 1) {
				dir = NORTH;
			}
			else {
				dir = WEST;
			}

			if (val >= max_value) {
				max_value = val;
				*max_index = index - N - i;
			}

			val = (val <= 0) ? 0 : val;
			dir = (val == 0) ? CENTER : dir;
			similarity_matrix[index] = val;
			direction_matrix[index - N - i] = dir;
			index++;
		}
		index++;
	} // end of for-loop
}
}


