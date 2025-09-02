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
    int north_1 = 0;
	int west_1 = 0;
	int northwest_1 = 0;
	int max_value = 0;
	int match_1 = 0;
	int index_1 = 0;
	int val_1 = 0;
	int test_val_1 = 0;
	int dir_1 = 0;

	int north_2 = 0;
	int west_2 = 0;
	int northwest_2 = 0;
	int match_2 = 0;
	int index_2 = 0;
	int val_2 = 0;
	int test_val_2 = 0;
	int dir_2 = 0;

    int buf1[N+1] = {0};
    int buf2[N+1] = {0};
    int buf3[N+1] = {0};
    int buf4[N+1] = {0};
    int buf5[N+1] = {0};
    query_t str1_buf[N];
    database_t str2_buf[N];


#pragma HLS ARRAY_PARTITION variable=str2_buf dim=1 complete
#pragma HLS ARRAY_PARTITION variable=str1_buf dim=1 complete
#pragma HLS ARRAY_PARTITION variable=buf1 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=buf2 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=buf3 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=buf4 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=buf5 dim=1 complete

    int max_index_buf = 0;
    int high = 0;
    int diags = N+M-1;
    int col;
    int row;

/*	for (int i=0; i<M+2*N-2; i++) {
	    	printf("%s", ((ap_uint<2>)string2[i]).to_string(10).c_str());
	    }
	printf("\n");*/
    memcpy(str1_buf, string1, N);

OUTER: for (i=0; i<diags; i++) {
#pragma HLS PIPELINE off
		row = high;
		memcpy(str2_buf, &string2[row], N);

INNER: for (col=N; col>=1; col=col-2) {
#pragma HLS PIPELINE II=1
        	
        	if ((row < N - 1) || (row >= M+N-1)) {
        		buf3[col] = ap_int<2>(0);
        	}
        	else {
				north_1 = buf2[col];
				west_1 = buf2[col-1];
				northwest_1 = buf1[col-1];

				//printf("buf_str2 =");
				/*for (int i=0; i<N; i++) {
				    	printf("%s", ((ap_uint<2>)str2_buf[i]).to_string(10).c_str());
				    }
				printf("\n");*/
				match_1 = (str1_buf[col-1]==str2_buf[row-i]) ? MATCH: MISS_MATCH;
				val_1 = (north_1 >= west_1) ? north_1 + GAP_d : west_1 + GAP_i;
				test_val_1 = northwest_1 + match_1;

				if (test_val_1 >= val_1) {
					val_1 = test_val_1;
					dir_1 = NORTH_WEST;
				}
				else if (val_1 == north_1-1) {
					dir_1 = NORTH;
				}
				else {
					dir_1 = WEST;
				}

				val_1 = (val_1 <= 0) ? 0 : val_1;
				dir_1 = (val_1 == 0) ? CENTER : dir_1;


				buf3[col] = val_1;

				index_1 = row - N + 1;
				if ((index_1 >= 0) &&  (index_1 < M)) {
					direction_matrix[col - 1 + index_1*N] = dir_1;
				}

				if (val_1>=buf4[col]) {
					buf4[col] = val_1;
					buf5[col] = col - 1 + index_1*N;
				}
        	}

        	if ((row + 1 < N - 1) || (row + 1 >= M + N - 1)) {
        		buf3[col-1] = ap_int<2>(0);
        	}
        	else {
				north_2 = buf2[col-1];
				west_2 = buf2[col-2];
				northwest_2 = buf1[col-2];

				/*printf("buf_str2 =");
				for (int i=0; i<N; i++) {
				    	printf("%s", ((ap_uint<2>)buf_str2[i]).to_string(10).c_str());
				    }
				printf("\n");*/
				match_2 = (str1_buf[col-2]==str2_buf[row+1-i]) ? MATCH: MISS_MATCH;
				val_2 = (north_2 >= west_2) ? north_2 + GAP_d : west_2 + GAP_i;
				test_val_2 = northwest_2 + match_2;

				if (test_val_2 >= val_2) {
					val_2 = test_val_2;
					dir_2 = NORTH_WEST;
				}
				else if (val_2 == north_2-1) {
					dir_2 = NORTH;
				}
				else {
					dir_2 = WEST;
				}

				val_2 = (val_2 <= 0) ? 0 : val_2;
				dir_2 = (val_2 == 0) ? CENTER : dir_2;


				buf3[col -1 ] = val_2;

				index_2 = row - N + 2;
				if ((index_2 >= 0) &&  (index_2 < M)) {
					direction_matrix[col - 2 + index_2*N] = dir_2;
				}

				if (val_2>=buf4[col-1]) {
					buf4[col - 1] = val_2;
					buf5[col - 1] = col - 2 + index_2*N;
				}
        	}
        	//}
            row+=2;
        }
        memcpy(buf1, buf2, (N+1)*sizeof(int));
        memcpy(buf2, buf3, (N+1)*sizeof(int));
        high++;
    } // end of for-loop

MAX_VAL:  for (int i=0; i<N+1; i++) {
#pragma HLS PIPELINE off
    	if (buf4[i]>=max_value) {
    		max_value = buf4[i];
    		max_index_buf = buf5[i];
    	}
    }
    memcpy(max_index, &max_index_buf, sizeof(int));
   for (int i=0; i<M; i++) {
    	for (int j=0; j<N; j++) {
    		printf("%3d", direction_matrix[j+i*N]);
    	}
    	printf("\n");
    }
    printf("\n");

} // end of function
}


