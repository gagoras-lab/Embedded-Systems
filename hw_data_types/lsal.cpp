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
	query_t *string1, database_t *string2,
	int *max_index, uint8_t *direction_matrix)
{
    int i = 0;

#pragma HLS INTERFACE m_axi port=string1 offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=string1 bundle=control
#pragma HLS INTERFACE m_axi port=string2 offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=string2 bundle=control
#pragma HLS INTERFACE m_axi port=max_index offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=max_index bundle=control
#pragma HLS INTERFACE m_axi port=direction_matrix offset=slave bundle=gmem
#pragma HLS INTERFACE s_axilite port=direction_matrix bundle=control

#pragma HLS INTERFACE s_axilite port=return bundle=control

    // Following values are used for the N, W, and NW values wrt. similarity_matrix[i]
    int north = 0;
	int west = 0;
	int northwest = 0;
	int max_value = 0;
	int match = 0;
	int index = 0;
	int val = 0;
	int test_val = 0;
	int dir = 0;

    int buf1[N+1] = {0};
    int buf2[N+1] = {0};
    int buf3[N+1] = {0};
    int buf4[N+1] = {0};
    int buf5[N+1] = {0};
    query_t str1_buf[N];
    database_t str2_buf[M+2*N-2];
    uint8_t direction_matrix_buf[N*M];

#pragma HLS ARRAY_PARTITION variable=str2_buf dim=1 complete
#pragma HLS ARRAY_PARTITION variable=str1_buf dim=1 complete
#pragma HLS ARRAY_PARTITION variable=buf1 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=buf2 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=buf3 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=buf4 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=buf5 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=direction_matrix_buf dim=1 complete

    int max_index_buf = 0;
    int high = 0;
    int diags = N+M-1;
    int col;
    int row;

    memcpy(str1_buf, string1, N);
    memcpy(str2_buf, string2, (M+2*N-2));

    /*printf("str2_buf\n");
    for (int i=0; i<M+2*N-2; i++) {
    	printf("%s", (str2_buf[i].range(1, 0)).to_string(10).c_str());
    }
    printf("\n");
    printf("string2\n");
        for (int i=0; i<M+2*N-2; i++) {
        	printf("%s", (string2[i].range(1, 0)).to_string(10).c_str());
       }
    printf("\n");*/
    //printf("str2_buf = %4s\n", str2_buf);
    //printf("string2 = %s\n", string2);
    //printf("N = %d\n", N);
    //printf("M = %d\n", M);
    for (i=0; i<diags; i++) {
#pragma HLS PIPELINE off
        row = high;
        for (col=N; col>=1; col--) {
#pragma HLS PIPELINE off
#pragma HLS UNROLL
        	if ((row < N - 1) || (row >= M+N-1)) {
        		buf3[col] = ap_int<2>(0);
        	}
        	else {
				north = buf2[col];
				west = buf2[col-1];
				northwest = buf1[col-1];

				match = (str1_buf[col-1]==str2_buf[row]) ? MATCH: MISS_MATCH;
				val = (north >= west) ? north + GAP_d : west + GAP_i;
				test_val = northwest + match;

				if (test_val >= val) {
					val = test_val;
					dir = NORTH_WEST;
				}
				else if (val == north-1) {
					dir = NORTH;
				}
				else {
					dir = WEST;
				}

				val = (val <= 0) ? 0 : val;
				dir = (val == 0) ? CENTER : dir;
				buf3[col] = val;
				index = row - N + 1;
				if ((index >= 0) &&  (index < M)) {
					direction_matrix_buf[col - 1 + index*N] = dir;
				}

				if (val>=buf4[col]) {
					buf4[col] = val;
					buf5[col] = col - 1 + index*N;
				}
        	}
            row++;
        }
        memcpy(buf1, buf2, (N+1)*sizeof(int));
        memcpy(buf2, buf3, (N+1)*sizeof(int));
        high++;
    } // end of for-loop

    for (int i=0; i<N+1; i++) {
#pragma HLS PIPELINE off
    	if (buf4[i]>=max_value) {
    		max_value = buf4[i];
    		max_index_buf = buf5[i];
    	}
    }
    memcpy(max_index, &max_index_buf, sizeof(int));
    memcpy(direction_matrix, direction_matrix_buf, N*M*sizeof(uint8_t));
  /*  for (int i=0; i<M; i++) {
    	for (int j=0; j<N; j++) {
    		printf("%3d", direction_matrix[j+i*N]);
    	}
    	printf("\n");
    }
    printf("\n");*/

} // end of function
}


