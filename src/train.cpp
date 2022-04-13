const char* help =
	"ad_train: train a model on a featurized dataset\n\n"
	
	"usage:\n"
	"  -i [input_path]: required, path to a featurized dataset";

#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <span>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <float.h>

#include "types.hpp"
#include "pack.hpp"

#define AD_FLAG_INPUT "-i"
#define AD_FLAG_HELP "-h"

typedef float32 (*ns_function)(uint32, uint32);

#define rand_float32(max) (static_cast<float32>(rand()) / static_cast<float32>(RAND_MAX / (max)))

struct vector_t {
	float32* data;
	uint32 size;
	
	float32& operator[](uint32 i) { return *(data + i); }
};

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

#define vec_for(v, e) for (auto e = v.data; e < v.data + v.size; e++)


struct matrix_t {
	float32* data;
	uint32 rows;	
	uint32 cols;
};

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

#define mtx_for(m, v) for (vector_t v = mtx_at(m, 0); v.data < m.data + (m.rows * m.cols); v.data = v.data + m.cols)

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

// Neighborhood strength functions
float32 ns_linear(uint32 winning_cluster, uint32 neighbor_cluster) {
	int32 max_distance = 2;
	float32 decay = .5;
	
	int32 distance = abs((int32)neighbor_cluster - (int32)winning_cluster);
	if (distance > max_distance) return 0;
	
	return 1 - (distance * decay);
}

float32 ns_none(uint32 winning_cluster, uint32 neighbor_cluster) {
	if (neighbor_cluster == winning_cluster) return 1;
	return 0;
}

// Algorithm parameters
ns_function ns = &ns_linear;
float32 learning_rate = .2;
uint32 clusters = 4;
float32 error_threshold = .01;

// Algorithm functions
uint32 find_winning_cluster(matrix_t& weights, vector_t& input) {
	uint32 winning_cluster = 0;
	float32 min_distance = FLT_MAX;
	
	mtx_for(weights, weight) {
		uint32 cluster = mtx_indexof(weights, weight);
		float32 distance = vec_distance(input, weight);
		if (min_distance > distance) {
			winning_cluster = cluster;
			min_distance = distance;
		}
	}

	return winning_cluster;
}

void update_weights(matrix_t& weights, vector_t& input, uint32 winning_cluster) {
	mtx_for(weights, weight) {
		uint32 cluster = mtx_indexof(weights, weight);
		float32 strength = ns(winning_cluster, cluster);
		for (int i = 0; i < input.size; i++) {
			weight[i] += learning_rate * strength * (input[i] - weight[i]);
		}
	}	
}

void calculate_weight_deltas(matrix_t& weights, vector_t& input, uint32 winning_cluster, matrix_t& deltas) {
	mtx_for(weights, weight) {
		uint32 cluster = mtx_indexof(weights, weight);
		float32 strength = ns(winning_cluster, cluster);
		for (int i = 0; i < input.size; i++) {
			*mtx_at(deltas, cluster, i) += learning_rate * strength * (input[i] - weight[i]);
		}
	}	
}

float32 squared_error(vector_t& weight, vector_t& input) {
	float32 error = 0;
	for (uint32 i = 0; i < input.size; i++) {
		error += pow(input[i] - weight[i], 2);
	}

	return error;
}


// SOM
void som_init(matrix_t& weights, matrix_t& inputs) {
	mtx_for(inputs, input) {
		vec_normalize(input);
	}

	mtx_for(weights, weight) {
		vec_for(weight, w) {
			*w = rand_float32(1);
		}
	}
	mtx_for(weights, weight) {
		vec_normalize(weight);
	}
}


int main(int arg_count, char** args) {
	char input_path [AD_PATH_SIZE] = { 0 };

	// Parse arguments and check for validity
	for (int32 i = 1; i < arg_count; i++) {
		char* flag = args[i];
		if (!strcmp(flag, AD_FLAG_INPUT)) {
			char* arg = args[++i];
			strncpy(input_path, arg, AD_PATH_SIZE);
		}
		else if (!strcmp(flag, AD_FLAG_HELP)) {
			printf("%s\n", help);
			exit(0);
		}
	}

	if (!strlen(input_path)) {
		printf("%s\n", help);
		exit(1);
	}

	// Load the binary input
	FILE* file = fopen(input_path, "r");
	if (!file) fprintf(stderr, "cannot open input file, path = %s\n", input_path);
	fseek(file, 0, SEEK_END);
	uint32 file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* buffer = (char*)calloc(sizeof(char), file_size);
	fread(buffer, file_size, 1, file);
	fclose(file);

	// Deserialize the binary input. It's serialized very simply -- just a header,
	// and then a tighly packed array of floats.
    ad_featurized_header* header = (ad_featurized_header*)buffer;
	uint32 rows = header->rows;
	uint32 cols = header->features_per_row;

	float32* input_data = (float32*)(buffer + sizeof(ad_featurized_header));

	//
	// SOM
	//
	srand(102);

	matrix_t inputs;
	mtx_init(&inputs, input_data, rows, cols);
	matrix_t weights;
	mtx_init(&weights, clusters, cols);
	matrix_t deltas;
	mtx_init(&deltas, clusters, cols);
	vector_t winners;
	vec_init(&winners, rows);

	som_init(weights, inputs);

	// Loop: Find each point's winning cluster, and then adjust this cluster and neighboring
	// clusters to be closer to this point. Break when MSE reaches a threshold.
	uint32 i = 0;
	float32 delta_error = error_threshold + 1;
	float32 error = FLT_MAX;
	float32 last_error = FLT_MAX;
	bool retry = true;
	uint32 jitter = 5;
	while (retry) {
		last_error = error;
		error = 0;

		mtx_for(inputs, input) {
			uint32 cluster = find_winning_cluster(weights, input);
			calculate_weight_deltas(weights, input, cluster, deltas);
			winners[mtx_indexof(inputs, input)] = cluster;
		}

		// Batch style update -- average weight deltas and apply them to weights
		mtx_scale(deltas, 1.f / mtx_size(deltas));
		mtx_add(weights, deltas);
		memset(deltas.data, 0, sizeof(float32) * mtx_size(deltas));

		// MSE between all inputs and their winning cluster
		mtx_for(inputs, input) {
			uint32 cluster = winners[mtx_indexof(inputs, input)];
			vector_t weight = mtx_at(weights, cluster);
			error += squared_error(weight, input);
		}
		delta_error = abs(error - last_error);

		printf("iteration = %d, error = %f\n", i++, error);

		if (delta_error < error_threshold && jitter) {
			printf("jittering\n");
			jitter--;
			mtx_for(weights, weight) {
				vec_for(weight, w) {
					*w += rand_float32(.1);
				}
			}
		}
		if (!jitter) retry = false;
	}

	for (uint32 i = 0; i < winners.size; i++) {
		printf("input %d: %d\n", i, (uint32)winners[i]);
	}
	mtx_for(weights, weight) {
		uint32 cluster = mtx_indexof(weights, weight);
		printf("cluster %d: (%f, %f, %f, %f)\n", cluster, weight[0], weight[1], weight[2], weight[3]);
	}
	printf("done\n");


	
	// C :: number of clusters
	// N :: points in training set
	// F :: number of features per point
	
	// initialize a learning rate, beta
	// initialize C sets of weights of dimension Fx1, eith each element 0 <= E <= 1
	// select a neighborhood strength function NS, which defines how to adjust weights
	//   of nodes adjacent to the winning node
	
	// for point in training_set
	//   for weight in weights
	//     calculate distance between point and weight
	//
	//   adjust winning weight by:
	//     w_j = w_j + beta * NS(input - w_j)
}
