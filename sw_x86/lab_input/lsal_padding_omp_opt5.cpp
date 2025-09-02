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
#include <omp.h>
#include <climits>

// #define N 256
// #define M 2048

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(a, b) ((a) > (b) ? a : b)
#define ABS(x) ((x) < 0) ? -(x) : x

const short GAP_i = -1;
const short GAP_d = -1;
const short MATCH = 2;
const short MISS_MATCH = -1;
const short CENTER = 0;
const short NORTH = 1;
const short NORTH_WEST = 2;
const short WEST = 3;
#define THREADS 1
// static long int cnt_ops=0;
// static long int cnt_bytes=0;

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
struct max_value {
	int value; 
	int index;
} max_struct;

#pragma omp declare reduction(max : struct max_value : omp_out = (omp_in.value > omp_out.value) ? omp_in : omp_out)

void compute_matrices(
	char *string1, char *string2,
	int *max_index, int *similarity_matrix, short *direction_matrix, int N, int M)
{

    // Following values are used for the N, W, and NW values wrt. similarity_matrix[i]
    //strcpy(string1, "TGTTACGG");
	//strcpy(string2, "GGTTGACTA");

	//Here the real computation starts. Place your code whenever is required. 
	//Scan the N*M array row-wise starting from the second row.
	int diags = M + N - 2;
	int diag_elem = 0;
	int i=0, j=0;
	int max = MAX(M, N);
	int min = MIN(M, N);
	int diff = M - N;
	int col= 0, row = 0;
	#pragma omp parallel shared(similarity_matrix, direction_matrix, diags, string1, string2) private(diag_elem, col, row) 
	{
		//#pragma omp for schedule(dynamic)
		for (i=1; i<= diags; i++) {
			
			if ((i<M) && (i<N)) {
				diag_elem = i;
			}
			else if (i < max) {
				diag_elem = min - 1;
			}
			else {
				diag_elem = (2*min) - i + abs(diff) - 1;
			}
			
			if (i < N) {
				col = i;
				row = 1;
			}
			else {
				col = N - 1;
				row = i - N + 1;
			}
	
		#pragma omp parallel for schedule(auto) num_threads(4) reduction(max:max_struct)
			for (j = 1; j <= diag_elem; j++) {

				int north = 0;
				int west = 0;
				int northwest = 0;
				int match = 0;
				int val = 0;
				int test_val = 0;
				int dir = 0;

				int index = col - 1  +  (row - 1) * N;
				northwest = similarity_matrix[index];
				west = similarity_matrix[index + N];
				north = similarity_matrix[index + 1];


				match = (string1[col-1]==string2[row-1]) ? MATCH : MISS_MATCH;
				val = ( north >= west) ? north + GAP_d : west + GAP_i;
				test_val = northwest + match;

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
				//#pragma omp critical
				//{
				if (val >= max_struct.value) {
					max_struct.value = val;
					max_struct.index = col + row*N;
				}
				val = (val <= 0) ? 0 : val;
				dir = (val == 0) ? CENTER : dir;
				similarity_matrix[col + row*N] = val;
				direction_matrix[col-1 + (row-1)*(N-1)] = dir;
				
				row++;
				col--;
			}
		}
	}

	*max_index = max_struct.index;
	// end of for-loop

#ifdef DEBUG
printf("String 1 = %s\n", string1);
printf("String 2 = %s\n", string2);

printf("-------Similarity Matrix-------\n");
for (int i=0; i<=M-1; i++) {
	for (int j=0; j<=N-1; j++) {
		printf("%3d ", similarity_matrix[j+i*N]);
	}
	printf("\n");
}

printf("\n\n\n");

printf("-------Direction Matrix-------\n");

for (int i=0; i<M-1; i++) {
	for (int j=0; j<N-1; j++) {
		switch (direction_matrix[j+i*(N-1)]) {
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

    double t1, t2;

	if (argc != 3) {
		printf("%s <Query Size N> <DataBase Size M>\n", argv[0]);
		return EXIT_FAILURE;
	}
  
    //printf("Starting Local Alignment Code \n");
	fflush(stdout);
	max_struct.value = INT_MIN;
	/* Typically, M >> N */
	int N = atoi(argv[1]); 
    int M = atoi(argv[2]);

    char *query = (char*) malloc(sizeof(char) * N);
	char *database = (char*) malloc(sizeof(char) * M);
	int *similarity_matrix = (int*) malloc(sizeof(int) * (N+1) * (M+1));
	short *direction_matrix = (short*) malloc(sizeof(short) * N * M);
	int *max_index = (int *) malloc(sizeof(int));

/* Create the two input strings by calling a random number generator */
	fillRandom(query, N);
	fillRandom(database, M);

	memset(similarity_matrix, 0, sizeof(int) * (N+1) * (M+1));
	memset(direction_matrix, 0, sizeof(short) * N * M);

    t1 = omp_get_wtime();
	compute_matrices(query, database, max_index, similarity_matrix, direction_matrix, N+1, M+1);
	t2 = omp_get_wtime();

	printf("%lf\n", t2-t1);
    //printf(" max index is in position (%d, %d) \n", max_index[0]/(N+1) - 1 , max_index[0]%(N+1) - 1 );

	free(similarity_matrix);
	free(direction_matrix);
	free(max_index);
	
	return EXIT_SUCCESS;
}
