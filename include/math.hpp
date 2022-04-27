#ifndef AD_MATH_H
#define AD_MATH_H

#include "types.hpp"

#define rand_float32(max) (static_cast<float32>(rand()) / static_cast<float32>(RAND_MAX / (max)))

struct vector_t {
	float32* data;
	uint32 size;
	
	float32& operator[](uint32 i) { return *(data + i); }
};

void vec_init(vector_t* vector, float32* data, uint32 size);
void vec_init(vector_t* vector, uint32 size);
float32 vec_length(vector_t& vec);
void vec_normalize(vector_t& vec);
float32 vec_distance(vector_t& va, vector_t& vb);
void vec_subtract(vector_t& va, vector_t& vb, vector_t& vout);
#define vec_for(v, e) for (auto e = v.data; e < v.data + v.size; e++)


struct matrix_t {
	float32* data;
	uint32 rows;	
	uint32 cols;
};

void mtx_init(matrix_t* mtx, uint32 rows, uint32 cols);
void mtx_init(matrix_t* mtx, float32* data, uint32 rows, uint32 cols);
vector_t mtx_at(matrix_t& mtx, uint32 row);
float32* mtx_at(matrix_t& mtx, uint32 row, uint32 col);
uint32 mtx_indexof(matrix_t& mtx, vector_t& vec);
void mtx_scale(matrix_t& mtx, float32 scalar);
void mtx_add(matrix_t& mtx, matrix_t& deltas);
uint32 mtx_size(matrix_t& mtx);
#define mtx_for(m, v) for (vector_t v = mtx_at(m, 0); v.data < m.data + (m.rows * m.cols); v.data = v.data + m.cols)

#endif