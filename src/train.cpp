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

void normalize_vector(float32* vector, uint32 size) {
	float32 acc = 0.f;
	for (uint32 i = 0; i < size; i++) acc += vector[i] * vector[i];
	acc = sqrt(acc);
	for (uint32 i = 0; i < size; i++) vector[i] /= acc;
}

float32 vec_distance(float32* va, float32* vb, uint32 dim) {
	float32 sum = 0;
	for (int i = 0; i < dim; i++) {
		sum = sum + pow(va[i] - vb[i], 2);
	}
	return sqrt(sum);	
}

void vec_subtract(float32* va, float32* vb, uint32 dim, float32* vout) {
	for (int i = 0; i < dim; i++) {
		vout[i] = va[i] - vb[i];
	}
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
	uint32 clusters = 3;
	float32 error_target = .1;

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
	uint32 input_size = header->rows * header->features_per_row;
	float32* input_data = (float32*)(buffer + sizeof(ad_featurized_header));


	// Randomly initialize weights between 0 and 1, then normalize them
	std::vector<float32> weights(clusters * header->features_per_row);
	for (auto& weight : weights) {
		weight = rand_float32(1);
	}

	for (int i = 0; i < weights.size(); i += header->features_per_row) {
		normalize_vector(weights.data() + i, header->features_per_row);
	}

	uint32 iteration = 0;
	float32 error = FLT_MAX;
	while (error > error_target) {
		iteration++;
		for (int i = 0; i < header->rows; i++) {
			float32* input = input_data + (i * header->features_per_row);
			uint32 winning_cluster = 0;
			float32 min_distance = UINT32_MAX;
			for (int c = 0; c < clusters; c++) {
				float32* weight = weights.data() + (c * header->features_per_row);
				float32 distance = vec_distance(input, weight, header->features_per_row);
				if (min_distance > distance) {
					min_distance = distance;
					winning_cluster = c;
				}
			}

			for (int c = 0; c < clusters; c++) {
				float32 strength = ns(winning_cluster, c);
				float32* weight = weights.data() + (c * header->features_per_row);
				for (int f = 0; f < header->features_per_row; f++) {
					weight[f] += learning_rate * strength * (input[f] - weight[f]);
				}
			}
		}

		// MSE of all inputs and winning clusters
		error = 0;
		for (int i = 0; i < header->rows; i++) {
			float32* input = input_data + (i * header->features_per_row);
			uint32 winning_cluster = 0;
			float32 min_distance = UINT32_MAX;
			for (int c = 0; c < clusters; c++) {
				float32* weight = weights.data() + (c * header->features_per_row);
				float32 distance = vec_distance(input, weight, header->features_per_row);
				if (min_distance > distance) {
					min_distance = distance;
					winning_cluster = c;
				}
			}

			float32* cluster = weights.data() + (winning_cluster * header->features_per_row);
			for (int f = 0; f < header->features_per_row; f++) {
				error += pow(input[f] - cluster[f], 2);
			}
		}

		printf("iteration = %d, error = %f\n", iteration, error);
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
