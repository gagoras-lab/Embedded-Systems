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
    char str1_buf[N];
    memset(str1_buf, 0, N);
//#pragma HLS ARRAY_PARTITION variable=str1_buf dim=1 complete
    char str2_buf[M+2*N-2];
    memset(str2_buf, 0, M+2*N-2);
//#pragma HLS ARRAY_RESHAPE variable=str2_buf dim=1 complete
    int max_index_buf = 0;
    int high = 0;
    int diags = N+M-1;
    int col;
    int row;

    memcpy(str1_buf, string1, N*sizeof(char));
    memcpy(str2_buf, string2, (M+2*N-2)*sizeof(char));

    /*printf("str2_buf = %4s\n", str2_buf);
    printf("string2 = %s\n", string2);
    printf("N = %d\n", N);
    printf("M = %d\n", M);*/
    for (i=0; i<diags; i++) {
#pragma HLS PIPELINE II=1
        row = high;

        for (col=N; col>=1; col--) {
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
                direction_matrix[col - 1 + index*N] = dir;
			}

            if (val>=buf4[col]) {
            	buf4[col] = val;
            	buf5[col] = col - 1 + index*N;
            }

            row++;
        }
        memcpy(buf1, buf2, (N+1)*sizeof(int));
        memcpy(buf2, buf3, (N+1)*sizeof(int));
        high++;
    } // end of for-loop

    for (int i=0; i<N+1; i++) {
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


