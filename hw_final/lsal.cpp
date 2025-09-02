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
	ap_int<2> *string1, ap_int<2> *string2,
	int *max_index, uint8_t *direction_matrix)
{

    int i = 0;


    // Following values are used for the N, W, and NW values wrt. similarity_matrix[i]

    short buf1[N+1] = {0};
#pragma HLS ARRAY_PARTITION variable=buf1 dim=1 complete
    short buf2[N+1] = {0};
#pragma HLS ARRAY_PARTITION variable=buf2 dim=1 complete
    short buf3[N+1] = {0};
#pragma HLS ARRAY_PARTITION variable=buf3 dim=1 complete
    short buf4[N] = {0};
#pragma HLS ARRAY_PARTITION variable=buf4 dim=1 complete
    int buf5[N] = {0};
#pragma HLS ARRAY_PARTITION variable=buf5 dim=1 complete
    int high = 0;
    int diags = N+M-1;
    int col;
    int row;
    int max_index_buf = 0;
    short max_value = 0;

    ap_int<2> str1_buf[N];
#pragma HLS ARRAY_PARTITION variable=str1_buf dim=1 complete
    ap_int<2> str2_buf[N];
#pragma HLS ARRAY_PARTITION variable=str2_buf dim=1 complete
    ap_int<2> str2_tmp[M+2*N-1];
    uint8_t direction_matrix_buf[N] = {0};
#pragma HLS ARRAY_PARTITION variable=direction_matrix_buf dim=1 complete

    memcpy(str1_buf, string1, N*sizeof(char));
    memcpy(str2_tmp, string2, (M+2*N-2)*sizeof(char));

    for (int k=0; k<N; k++) {
#pragma HLS PIPELINE
		str2_buf[k] = str2_tmp[k];
	}
   OUTER: for (i=0; i<diags; i++) {
#pragma HLS PIPELINE II=1
        row = high;

    INNER: for (col=N; col>=1; col--) {

        	short north;
        	short west;
        	short northwest;
        	short match;
        	short val = 0;
        	short tmp_val;
        	short test_val;
        	short dir = CENTER;
        	short test_dir;
        	short north_val;
        	short west_val;

        	if ((row < N - 1) || (row >= M + N - 1)) {
        		buf3[col] = ap_int<2>(0);
        		val = 0;
        	}
        	else {
				north = buf2[col];
				west = buf2[col-1];
				northwest = buf1[col-1];

				match = (str1_buf[col-1]==str2_buf[row-i]) ? MATCH: MISS_MATCH;

				north_val = north + GAP_d;
				west_val = west + GAP_i;
				test_val = northwest + match;

				if (north>=west) {
					tmp_val = north_val;
					test_dir = NORTH;
				}
				else {
					test_dir = WEST;
					tmp_val = west_val;
				}

				if (test_val >= tmp_val) {
					val = test_val;
					dir = NORTH_WEST;
				}
				else {
					val = tmp_val;
					dir = test_dir;
				}

				buf3[col] = (val <= 0) ? 0 :  val;
				direction_matrix_buf[col - 1] = (val <= 0) ? CENTER : dir;

				if (val >= buf4[col-1]) {
					buf4[col-1] = val;
					buf5[col-1] = col - 1 + (row - N + 1)*N;
				}
        	}
            row++;
        }
        memcpy(buf1, buf2, (N+1)*sizeof(short));
        memcpy(buf2, buf3, (N+1)*sizeof(short));
        memcpy(direction_matrix+i*N, direction_matrix_buf, N*sizeof(uint8_t));
        high++;
        for (int k=1; k<N; k++) {
#pragma HLS PIPELINE
        	str2_buf[k-1] = str2_buf[k];
        }
        str2_buf[N-1] = str2_tmp[high+N-1];
    } // end of for-loop

   for (int i=0; i<N; i++) {
#pragma HLS PIPELINE
	   if (buf4[i] >= max_value) {
			max_value = buf4[i];
			max_index_buf = buf5[i];
	   }
   }

   memcpy(max_index, &max_index_buf, sizeof(int));

    /*for (int i=0; i<diags; i++) {
    	for (int j=0; j<N; j++) {
    		printf("%3d", direction_matrix[j+i*N]);
    	}
    	printf("\n");
    }
    printf("\n");*/

} // end of function
}


