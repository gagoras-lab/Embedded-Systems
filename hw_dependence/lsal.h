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

#define N 10
#define M 20

typedef ap_int<2> query_t;
typedef ap_int<2> database_t;

extern "C" {
	void compute_matrices(
    query_t *string1, database_t *string2,
    int *max_index, uint8_t *direction_matrix);
}

#endif
