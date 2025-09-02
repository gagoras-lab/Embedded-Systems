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

extern "C" {
void compute_matrices_sw(
	char *string1, char *string2,
	int *max_index, int *similarity_matrix, uint8_t *direction_matrix)
{

	int index = 0;
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
	   for(index = N; index < N*(M+1); index++) {
		i = index % N; // column index
			j = index / N - 1; // row index

		if (i==0) {
			if (j==0) {
				north = 0;
			}
			else {
				north = similarity_matrix[i+(j-1)*N];
			}
			west = 0;
			northwest = 0;
		}
		else if (j==0) {
			north = 0;
			west = similarity_matrix[i-1+j*N];
			northwest = 0;
		}
		else {
			west = similarity_matrix[i-1+j*N];
			north = similarity_matrix[i+(j-1)*N];
			northwest = similarity_matrix[i-1 + (j-1)*N];
		}

		match = (string1[i]==string2[j]) ? MATCH : MISS_MATCH;
		val = ( north >= west) ? north + GAP_d : west + GAP_i;
		test_val = northwest + match;

		if (test_val >= val) {
			val = test_val;
			dir = 2;
		}
		else if (val == north - 1) {
			dir = 1;
		}
		else {
			dir = 3;
		}


		val = (val <= 0) ? 0 : val;

		if (val >= max_value) {
			max_value = val;
			*max_index = i+j*N;
		}

		dir = (val == 0) ? 0 : dir;
		similarity_matrix[i+j*N] = val;
		direction_matrix[i+j*N] = dir;

	}
	   // end of for-loop
	/*   for (int i=0; i<M; i++) {
	    	for (int j=0; j<N; j++) {
	    		printf("%3d", direction_matrix[j+i*N]);
	    	}
	    	printf("\n");
	    }
	    printf("\n");*/
}  // end of function

int rand_lim(int limit) {

	int divisor = RAND_MAX / (limit + 1);
	int retval;

	do {
		retval = rand() / divisor;
	} while (retval > limit);

	return retval;
}

/*
 Fill the string with random values
 */
void fillRandom(char* string, int dimension) {
	//fill the string with random letters..

	string[0] = '-';

	int i;
	for (i = 0; i < dimension; i++) {
		int randomNum = rand() % 4;
		string[i] = randomNum;
	}

}


int main(int argc, char** argv) {
	

    printf("Starting Local Alignment Code \n");
	fflush(stdout);
    
	int *max_index = (int *) malloc(sizeof(int));
    uint8_t *direction_matrix = (uint8_t *) malloc(sizeof(uint8_t) * N * M);
    memset(direction_matrix, 0, sizeof(uint8_t)*N*M);
    
    int *max_index_sw = (int *) malloc(sizeof(int));
    int *similarity_matrix_sw = (int *)malloc(sizeof(int)*N*M);
    uint8_t *direction_matrix_sw = (uint8_t *) malloc(sizeof(uint8_t) * N * M);
    memset(similarity_matrix_sw, 0, sizeof(int)*N*M);
    memset(direction_matrix_sw, 0, sizeof(uint8_t)*N*M);
    
    char *query_sw = (char *)malloc(sizeof(char)*N);
    char *database_sw = (char *)malloc(sizeof(char)*M);

    query_t query[N];
	database_t database[M+2*N-2];
    fillRandom(query_sw, N);
	fillRandom(database_sw, M);
    
	for (int i=0; i<N; i++) {
		query[i] = ap_int<2>(query_sw[i]);
	}

    for (int i=0; i<N-1; i++) {
        database[i] = ap_int<2>(0);
    }
    
    for (int i=N-1; i<M+(N-1); i++) {
    	char *tmp = (char*)malloc(1);
    	sprintf(tmp, "%d", database_sw[i-N+1]);
        database[i] = ap_int<2>(tmp, 10);
    }

    for (int i = M+N-1; i<M+2*N - 2; i++) {
    	database[i] = ap_int<2>(0);
    }

    int matrix_size = N*M;

	compute_matrices(query, database, max_index, direction_matrix);
	compute_matrices_sw(query_sw, database_sw, max_index_sw, similarity_matrix_sw, direction_matrix_sw);

	for (int i=0; i<matrix_size; i++) {
            if (direction_matrix[i] != direction_matrix_sw[i]) {
                printf("Error, mismatch at direction matrix at index %d, HW: %d, SW: %d\n", i, direction_matrix[i], direction_matrix_sw[i]);
                exit(EXIT_FAILURE);
			}
	}

	if (max_index[0] != max_index_sw[0]) {
			printf("Error, mismatch in max_index, HW: %d, SW: %d\n", max_index[0], max_index_sw[0]);
			exit(EXIT_FAILURE);
		}

	free(direction_matrix);
    free(query_sw);
    free(database_sw);
    free(similarity_matrix_sw);
	free(direction_matrix_sw);
	free(max_index);
	free(max_index_sw);

	return EXIT_SUCCESS;

}

}

