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

#include "types.hpp"
#include "pack.hpp"

#define AD_FLAG_INPUT "-i"
#define AD_FLAG_HELP "-h"

typedef float32 (*ns_function)(uint32, uint32);

#define rand_float32(max) (static_cast<float32>(rand()) / static_cast<float32>(RAND_MAX / (max)))

void normalize_vector(float32* vector, uint32 size) {
	float32 acc = 0.f;
	for (uint32 i = 0; i < size; i++) acc += vector[i] * vector[i];
	acc = sqrt(acc);
	for (uint32 i = 0; i < size; i++) vector[i] /= acc;
}

float32 vec_distance(float32* va, uint32 sa, float32* vb, uint32 sb) {
	return 0.f;	
}

// Neighborhood strength functions
float32 ns_linear(uint32 winning_cluster, uint32 neighbor_cluster) {
	int32 max_distance = 2;
	float32 decay = .3;
	
	int32 distance = abs((int32)neighbor_cluster - (int32)winning_cluster);
	if (distance > max_distance) return 0;
	
	return 1 - (distance * decay);
}

int main(int arg_count, char** args) {
	char input_path [AD_PATH_SIZE] = { 0 };
	
	ns_function ns = &ns_linear;
	float32 learning_rate = .5;
	uint32 clusters = 2;

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

	FILE* file = fopen(input_path, "r");
	fseek(file, 0, SEEK_END);
	uint32 file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* buffer = (char*)calloc(sizeof(char), file_size);
	fread(buffer, file_size, 1, file);
	fclose(file);

	// Deserialize the binary input. It's serialized very simply -- just a header,
	// and then a tighly packed array of floats.
    ad_featurized_header* header = (ad_featurized_header*)buffer;
	uint32 input_size = header->rows * header->features_per_row;
	std::span<float32> input((float32*)(buffer + sizeof(ad_featurized_header)), input_size);


	// Randomly initialize weights between 0 and 1, then normalize them
	std::vector<float32> weights(clusters * header->features_per_row);
	for (auto& weight : weights) {
		weight = rand_float32(1);
	}

	for (int i = 0; i < weights.size(); i += header->features_per_row) {
		normalize_vector(weights.data() + i, header->features_per_row);
	}

	// Normalize the input data 
	for (int i = 0; i < input_size; i += header->features_per_row) {
		normalize_vector(input.data() + i, header->features_per_row);
	}

	for (int i = 0; i < header->rows; i++) {
		uint32 winning_cluster = 0;
		uint32 min_distance = UINT32_MAX;
		for (int c = 0; c < clusters; c++) {
			float32* w = weights.data() + (c * header->features_per_row);
			// calc distance
			// if smaller than min, this cluster is winning
		}

		for (int c = 0; c < clusters; c++) {
			// calculate NS between c and winning_cluster
			// update weight by beta * NS * ()
		}
	}


	
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
