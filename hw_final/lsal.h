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
#include "ap_int.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(a, b) ((a) > (b) ? a : b)
#define ABS(x) ((x) < 0) ? -(x) : x

#define N 64
#define M 65536


extern "C" {
	void compute_matrices(
    ap_int<2> *string1, ap_int<2> *string2,
    int *max_index, uint8_t *direction_matrix);
}

#endif
