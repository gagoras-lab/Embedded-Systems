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
    short north_1 = 0;
    short west_1 = 0;
    short northwest_1 = 0;
    short max_value = 0;
	short match_1 = 0;
	int index_1 = 0;
	short val_1 = 0;
	short test_val_1 = 0;
	short dir_1 = 0;

	short north_2 = 0;
	short west_2 = 0;
	short northwest_2 = 0;
	short match_2 = 0;
	int index_2 = 0;
	short val_2 = 0;
	short test_val_2 = 0;
	short dir_2 = 0;

	short north_3 = 0;
	short west_3 = 0;
	short northwest_3 = 0;
	short match_3 = 0;
	int index_3 = 0;
	short val_3 = 0;
	short test_val_3 = 0;
	short dir_3 = 0;

	short north_4 = 0;
	short west_4 = 0;
	short northwest_4 = 0;
	short match_4 = 0;
	int index_4 = 0;
	short val_4 = 0;
	short test_val_4 = 0;
	short dir_4 = 0;

	short buf1[N+1] = {0};
	short buf2[N+1] = {0};
	short buf3[N+1] = {0};
    query_t str1_buf[N];
    database_t str2_buf[M+2*N-2];
    database_t str2_comp[N];
    uint8_t direction_matrix_buf[N] = {0};

#pragma HLS ARRAY_PARTITION variable=str2_comp dim=1 cyclic factor=4
#pragma HLS ARRAY_PARTITION variable=str1_buf dim=1 cyclic factor=4
#pragma HLS ARRAY_PARTITION variable=buf1 dim=1 cyclic factor=4
#pragma HLS ARRAY_PARTITION variable=buf2 dim=1 cyclic factor=4
#pragma HLS ARRAY_PARTITION variable=buf3 dim=1 cyclic factor=4
#pragma HLS ARRAY_PARTITION variable=direction_matrix_buf dim=1 cyclic factor=4

/*#pragma HLS ARRAY_PARTITION variable=str2_comp dim=1 complete
#pragma HLS ARRAY_PARTITION variable=str1_buf dim=1 complete
#pragma HLS ARRAY_PARTITION variable=buf1 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=buf2 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=buf3 dim=1 complete
#pragma HLS ARRAY_PARTITION variable=direction_matrix_buf dim=1 complete*/

    int max_index_buf = 0;
    int high = 0;
    int diags = N+M-1;
    int col;
    int row;

    memcpy(str1_buf, string1, N);
    memcpy(str2_buf, string2, M+2*N-2);
    //memcpy(str2_comp, str2_buf, N);
    /*for (int i=0; i<M+2*N-2; i++) {
		printf("%s", ((ap_uint<2>)str2_buf[i]).to_string(10).c_str());
	}
	printf("\n");*/
OUTER: for (i=0; i<diags; i++) {
//#pragma HLS PIPELINE II=1
#pragma HLS PIPELINE off
		row = high;
		memcpy(str2_comp, &str2_buf[row], N);

INNER: for (col=N; col>=1; col=col-4) {
#pragma HLS PIPELINE II=1
        	
        	if ((row < N - 1) || (row >= M+N-1)) {
        		buf3[col] = ap_int<2>(0);
        		val_1 = 0;
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
				match_1 = (str1_buf[col-1]==str2_comp[row-i]) ? MATCH: MISS_MATCH;
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

				//index_1 = row - N + 1;
				//if ((index_1 >= 0) &&  (index_1 < M)) {
					direction_matrix_buf[col - 1] = dir_1;
				//}

				/*if (val_1>=buf4[col]) {
					buf4[col] = val_1;
					buf5[col] = col - 1 + index_1*N;
				}*/
        	}

        	if ((row + 1 < N - 1) || (row + 1 >= M + N - 1)) {
        		buf3[col-1] = ap_int<2>(0);
        		val_2 = 0;
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
				match_2 = (str1_buf[col-2]==str2_comp[row+1-i]) ? MATCH: MISS_MATCH;
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


				buf3[col - 1] = val_2;

				//index_2 = row - N + 2;
				//if ((index_2 >= 0) &&  (index_2 < M)) {
				direction_matrix_buf[col - 2] = dir_2;
				//}

				/*if (val_2>=buf4[col-1]) {
					buf4[col - 1] = val_2;
					buf5[col - 1] = col - 2 + index_2*N;
				}*/

        	}

        	int max_value1 = 0;
			int max_index1 = 0;


			if (val_1>val_2) {
				max_value1 = val_1;
				max_index1 = col - 1 +(row-N+1)*N;
			}
			else {
				max_value1 = val_2;
				max_index1 = col - 2 +(row-N+2)*N;
			}

        	if ((row + 2 < N - 1) || (row + 2 >= M + N - 1)) {
				buf3[col-2] = ap_int<2>(0);
				val_3 = 0;
			}
			else {
				north_3 = buf2[col-2];
				west_3 = buf2[col-3];
				northwest_3 = buf1[col-3];

				/*printf("buf_str2 =");
				for (int i=0; i<N; i++) {
						printf("%s", ((ap_uint<2>)buf_str2[i]).to_string(10).c_str());
					}
				printf("\n");*/
				match_3 = (str1_buf[col-3]==str2_comp[row+2-i]) ? MATCH: MISS_MATCH;
				val_3 = (north_3 >= west_3) ? north_3 + GAP_d : west_3 + GAP_i;
				test_val_3 = northwest_3 + match_3;

				if (test_val_3 >= val_3) {
					val_3 = test_val_3;
					dir_3 = NORTH_WEST;
				}
				else if (val_3 == north_3-1) {
					dir_3 = NORTH;
				}
				else {
					dir_3 = WEST;
				}

				val_3 = (val_3 <= 0) ? 0 : val_3;
				dir_3 = (val_3 == 0) ? CENTER : dir_3;


				buf3[col -2] = val_3;

				direction_matrix_buf[col - 3] = dir_3;

			}

        	if ((row + 3 < N - 1) || (row + 3 >= M + N - 1)) {
				buf3[col-3] = ap_int<2>(0);
				val_4 = 0;
			}
			else {
				north_4 = buf2[col-3];
				west_4 = buf2[col-4];
				northwest_4 = buf1[col-4];

				/*printf("buf_str2 =");
				for (int i=0; i<N; i++) {
						printf("%s", ((ap_uint<2>)buf_str2[i]).to_string(10).c_str());
					}
				printf("\n");*/
				match_4 = (str1_buf[col-4]==str2_comp[row+3-i]) ? MATCH: MISS_MATCH;
				val_4 = (north_4 >= west_4) ? north_4 + GAP_d : west_4 + GAP_i;
				test_val_4 = northwest_4 + match_4;

				if (test_val_4 >= val_4) {
					val_4 = test_val_4;
					dir_4 = NORTH_WEST;
				}
				else if (val_4 == north_4-1) {
					dir_4 = NORTH;
				}
				else {
					dir_4 = WEST;
				}

				val_4 = (val_4 <= 0) ? 0 : val_4;
				dir_4 = (val_4 == 0) ? CENTER : dir_4;


				buf3[col-3] = val_4;

				direction_matrix_buf[col - 4] = dir_4;

			}


        	int max_value2 = 0;
        	int max_index2 = 0;

        	if (val_3>val_4) {
				max_value2 = val_3;
				max_index2 = col - 3 +(row-N+3)*N;
			}
			else {
				max_value2 = val_4;
				max_index2 = col - 4 +(row-N+4)*N;
			}
        	if (max_value1 > max_value2) {
        		if (max_value1 >= max_value) {
        			max_value = max_value1;
        			max_index_buf = max_index1;
        		}
        	}
        	else {
        		if (max_value2 >= max_value) {
        			max_value = max_value2;
        			max_index_buf = max_index2;
        		}
        	}
        	//}
            row+=4;
        }
        memcpy(buf1, buf2, (N+1)*sizeof(short));
        memcpy(buf2, buf3, (N+1)*sizeof(short));
        memcpy(direction_matrix+i*N, direction_matrix_buf, N*sizeof(uint8_t));
        high++;
        /*memmove(str2_comp,str2_comp+1, N-1);
        str2_comp[N-1] = str2_buf[high + N-1];*/
    } // end of for-loop

/*MAX_VAL:  for (int i=0; i<N+1; i++) {
#pragma HLS PIPELINE off
    	if (buf4[i]>=max_value) {
    		max_value = buf4[i];
    		max_index_buf = buf5[i];
    	}
    }*/
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


