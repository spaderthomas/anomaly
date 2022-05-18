#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <cmath>
#include <cstdlib>
#include <cassert>
#include <vector>
#include <float.h>
#include <memory>
#include "math.hpp"

void vec_init(vector_t* vector, float32* data, uint32 size) {
	vector->data = data;
	vector->size = size;
}

void vec_init(vector_t* vector, uint32 size) {
	vector->data = (float32*)calloc(sizeof(float32), size);
	vector->size = size;
}

float32 vec_length(vector_t& vec) {
	float32 length = 0.f;
	for (uint32 i = 0; i < vec.size; i++) length += vec[i] * vec[i];
	length = sqrt(length);
	return length;
}

void vec_normalize(vector_t& vec) {
	float32 length = vec_length(vec);
	for (uint32 i = 0; i < vec.size; i++) vec[i] /= length;
}

float32 vec_distance(vector_t& va, vector_t& vb) {
	assert(va.size == vb.size);

	float32 sum = 0;
	for (int i = 0; i < va.size; i++) {
		sum = sum + pow(va[i] - vb[i], 2);
	}
	return sqrt(sum);	
}

void vec_subtract(vector_t& va, vector_t& vb, vector_t& vout) {
	assert(va.size == vb.size);
	for (int i = 0; i < va.size; i++) {
		vout[i] = va[i] - vb[i];
	}
}

vector_t vec_copy(vector_t& v) {
	vector_t copy;
	vec_init(&copy, v.size);
	memcpy(copy.data, v.data, v.size * sizeof(float32));
	return copy;
}

void vec_swap(vector_t& a, vector_t& b) {
	assert(a.size == b.size);
	vector_t copy = vec_copy(a);
	memcpy(a.data, b.data, a.size * sizeof(float32));
	memcpy(b.data, copy.data, b.size * sizeof(float32));
	vec_free(copy);
}

void vec_free(vector_t& v) {
	free(v.data);
	v.data = nullptr;
	v.size = 0;
}

void mtx_init(matrix_t* mtx, uint32 rows, uint32 cols) {
	mtx->data = (float32*)calloc(sizeof(float32), rows * cols);
	mtx->rows = rows;
	mtx->cols = cols;
}

void mtx_init(matrix_t* mtx, float32* data, uint32 rows, uint32 cols) {
	mtx->data = data;
	mtx->rows = rows;
	mtx->cols = cols;
}

vector_t mtx_at(matrix_t& mtx, uint32 row) {
	vector_t vec;
	vec.data = mtx.data + (row * mtx.cols);
	vec.size = mtx.cols;
	return vec;
}

float32* mtx_at(matrix_t& mtx, uint32 row, uint32 col) {
	return mtx.data + (row * mtx.cols) + col;
}

uint32 mtx_indexof(matrix_t& mtx, vector_t& vec) {
	assert(vec.data >= mtx.data);
	uint32 elements = vec.data - mtx.data;
	return elements / mtx.cols;
}

void mtx_scale(matrix_t& mtx, float32 scalar) {
	mtx_for(mtx, row) {
		vec_for(row, entry) {
			*entry *= scalar;
		}
	}
}

void mtx_add(matrix_t& mtx, matrix_t& deltas) {
	for (int i = 0; i < mtx.rows; i++) {
		for (int j = 0; j < mtx.cols; j++) {
			*mtx_at(mtx, i, j) += *mtx_at(deltas, i, j);
		}
	}
}
					 
uint32 mtx_size(matrix_t& mtx) {
	return mtx.rows * mtx.cols;
}