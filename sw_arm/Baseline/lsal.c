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

//#define N 256
//#define M 2048

const short GAP_i = -1;
const short GAP_d = -1;
const short MATCH = 2;
const short MISS_MATCH = -1;
const short CENTER = 0;
const short NORTH = 1;
const short NORTH_WEST = 2;
const short WEST = 3;
static long int cnt_ops=0;
static long int cnt_bytes=0;

/**********************************************************************************************
 * LSAL algorithm
 * Inputs:
 *          string1 is the query[N]
 *          string2 is the database[M]
 *          input sizes N, M
 * Outputs:
 *           max_index is the location of the highest similiarity score 
 *           similarity and direction matrices. Note that these two matrices are initialized with zeros.
 **********************************************************************************************/

void compute_matrices(
	char *string1, char *string2,
	int *max_index, int *similarity_matrix, short *direction_matrix, int N, int M)
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
	cnt_ops = 3;
for(index = N; index < N*(M+1); index++) { 

	cnt_ops += 1;
	cnt_ops+=2;
	i = index % N; // column index
		j = index / N - 1; // row index

	cnt_ops+=1;
	if (i==0) {
		cnt_ops+=1;
		if (j==0) {
			north = 0;
		}
		else {
			cnt_ops+=3;
			cnt_bytes+=4;
			north = similarity_matrix[i+(j-1)*N];
		}
		west = 0;
		northwest = 0;
	}
	else if (j==0) {
		cnt_ops+=1;
		north = 0;
		cnt_ops += 3;
		cnt_bytes+=4;
		west = similarity_matrix[i-1+j*N];
		northwest = 0;
	}
	else {
		cnt_ops+=1;
		cnt_bytes+=12;
		cnt_ops+=10;
		west = similarity_matrix[i-1+j*N];
		north = similarity_matrix[i+(j-1)*N];
		northwest = similarity_matrix[i-1 + (j-1)*N];
	}
	
	cnt_ops+=1;
	cnt_bytes+=2;
	match = (string1[i]==string2[j]) ? MATCH : MISS_MATCH;
	cnt_ops+=2;
	val = ( north >= west) ? north + GAP_d : west + GAP_i;
	cnt_ops+=1;
	test_val = northwest + match;

	cnt_ops+=1;
	if (test_val >= val) {
		val = test_val;
		dir = NORTH_WEST;
	}
	else if (val == north - 1) {
		cnt_ops+=2;
		dir = NORTH;
	}
	else {
		cnt_ops+=2;
		dir = WEST;
	}

	cnt_ops+=1;
	if (val > max_value) {
		max_value = val;
		cnt_bytes+=4;
		*max_index = index;
	}

	cnt_ops+=6;
	cnt_bytes+=6;
	val = (val <= 0) ? 0 : val;
	dir = (val == 0) ? CENTER : dir;
	similarity_matrix[i+j*N] = val;
	direction_matrix[i+j*N] = dir;

	cnt_ops += 4;
}   // end of for-loop

#ifdef DEBUG
printf("String 1 = %s\n", string1);
printf("String 2 = %s\n", string2);

printf("-------Similarity Matrix-------\n");
for (int i=0; i<M; i++) {
	for (int j=0; j<N; j++) {
		printf("%3d ", similarity_matrix[j+i*N]);
	}
	printf("\n");
}

printf("\n\n\n");

printf("-------Direction Matrix-------\n");

for (int i=0; i<M; i++) {
	for (int j=0; j<N; j++) {
		switch (direction_matrix[j+i*N]) {
			case(CENTER): {
				printf("C   ");
			}
			break;
			case(NORTH): {
				printf("N   ");
			}
			break;
			case(WEST): {
				printf("W   ");
			}
			break;
			case(NORTH_WEST): {
				printf("NW  ");
			}
			break;
		}
		
	}
	printf("\n");
	}
#endif
}  // end of function

/************************************************************************/

/*
return a random number in [0, limit].
*/
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
	static const char possibleLetters[] = "ATCG";

	string[0] = '-';

	int i;
	for (i = 0; i < dimension; i++) {
		int randomNum = rand_lim(3);
		string[i] = possibleLetters[randomNum];
	}

}

/* ******************************************************************/
int main(int argc, char** argv) {

	clock_t t1, t2;
	//FILE *ops = fopen("ops.out", "a");
	//FILE *bytes = fopen("bytes.out", "a");
	if (argc != 3) {
		printf("%s <Query Size N> <DataBase Size M>\n", argv[0]);
		return EXIT_FAILURE;
	}
	//printf("Starting Local Alignment Code \n");

	/* Typically, M >> N */
	int N = atoi(argv[1]); 
	int M = atoi(argv[2]);

	char *query = (char*) malloc(sizeof(char) * N);
	char *database = (char*) malloc(sizeof(char) * M);
	int *similarity_matrix = (int*) malloc(sizeof(int) * N * M);
	short *direction_matrix = (short*) malloc(sizeof(short) * N * M);
	int *max_index = (int *) malloc(sizeof(int));

/* Create the two input strings by calling a random number generator */
	fillRandom(query, N);
	fillRandom(database, M);

	memset(similarity_matrix, 0, sizeof(int) * N * M);
	memset(direction_matrix, 0, sizeof(short) * N * M);
	
	t1 = clock();
	compute_matrices(query, database, max_index, similarity_matrix, direction_matrix, N, M);
	t2 = clock();

	printf("%lf\n", (double)(t2-t1) / CLOCKS_PER_SEC);
//  printf(" max index is in position (%d, %d) \n", max_index[0]/N-1, max_index[0]%N );
	//printf(" execution time of Smith Waterman SW algorithm is %f sec \n", (double)(t2-t1) / CLOCKS_PER_SEC);

	printf("cnt_ops=%ld, cnt_bytes=%ld\n", cnt_ops, cnt_bytes);
	return EXIT_SUCCESS;
}
