#ifndef LSAL_H_
#define LSAL_H_

#define GAP_i -1
#define GAP_d -1
#define MATCH 2
#define MISS_MATCH -1
#define CENTER 0
#define NORTH 1
#define NORTH_WEST 2
#define WEST 3
#include <cstdint>

#define N 256
#define M 65536


extern "C" {
	void compute_matrices(
    char *string1, char *string2,
    int *max_index, uint8_t *direction_matrix);
}

#endif
