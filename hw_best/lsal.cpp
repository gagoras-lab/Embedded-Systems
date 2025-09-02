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

    int buf1[N+1] = {0};
    int buf2[N+1] = {0};
    int buf3[N+1] = {0};
    int buf4[N] = {0};
    int buf5[N] = {0};
    int high = 0;
    int diags = N+M-1;
    int col;
    int row;
    int max_index_buf = 0;
    int max_value = 0;

    char str1_buf[N];
    char str2_buf[N];
    char str2_tmp[M+2*N-2];
    uint8_t direction_matrix_buf[N] = {0};

    memcpy(str1_buf, string1, N*sizeof(char));
    memcpy(str2_tmp, string2, (M+2*N-2)*sizeof(char));

   OUTER: for (i=0; i<diags; i++) {
#pragma HLS PIPELINE II=1
        row = high;
        for (int k=1; k<N-1; k++) {
        	str2_buf[k-1] = str2_buf[k];
        }
        str2_buf[N-1] = str2_tmp[high+N-1];

    INNER: for (col=N; col>=1; col--) {
        	short north;
        	short west;
        	short northwest;
        	short match;
        	short val;
        	short test_val;
        	short dir;

			north = buf2[col];
			west = buf2[col-1];
			northwest = buf1[col-1];

			match = (str1_buf[col-1]==str2_buf[row-i]) ? MATCH: MISS_MATCH;
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
			direction_matrix_buf[col - 1] = dir;

			if (val >= buf4[col-1]) {
				buf4[col-1] = val;
				buf5[col-1] = col - 1 + (row - N + 1)*N;
			}
            row++;
        }
        memcpy(buf1, buf2, (N+1)*sizeof(int));
        memcpy(buf2, buf3, (N+1)*sizeof(int));
        memcpy(direction_matrix+i*N, direction_matrix_buf, N*sizeof(uint8_t));
        high++;
    } // end of for-loop

   for (int i=0; i<N; i++) {
#pragma HLS PIPELINE II=1
	   if (buf4[i]>=max_value) {
			max_value = buf4[i];
			max_index_buf = buf5[i];
	   }
   }

    memcpy(max_index, &max_index_buf, sizeof(int));


   /* for (int i=0; i<M; i++) {
    	for (int j=0; j<N; j++) {
    		printf("%3d", direction_matrix[j+i*N]);
    	}
    	printf("\n");
    }
    printf("\n");*/

} // end of function
}


