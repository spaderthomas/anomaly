// todo:
// - randomize the inputs; otherwise, you're feeding it all of the IRIS0 points at once, then all of the IRIS1, etc
// - learning rate decay -- begin taking smaller steps. optimize for error, not just clusters
//   - exponential decay
// - use exponential decay for neighborhood distance
// - jittering weights less common than changing initial weights
// - mess with iris dataset -- remove random datapoints, remove a cluster
//
// - think of this as a data pipeline
// - filenames: manually "cluster" them, run som, grab a few datapoints from the filename cluster and see how some classifies them

// gui is able to recompile these programs
// edit the parameters with Input**
//   neighborhood function, learning rate, # clusters, error threshold, seed
// write a parameter configuration out to a config file
// run button

const char* help =
	"ad_train: train a model on a featurized dataset\n\n"
	
	"usage:\n"
	"  -i [input_path]: required, path to a config file";

#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <cmath>
#include <cstdlib>
#include <vector>
#include <float.h>

#include "types.hpp"
#include "pack.hpp"
#include "math.hpp"
#include "som.hpp"

#define AD_FLAG_INPUT "-i"
#define AD_FLAG_HELP "-h"

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

	som_t som;
	cfg_load(&som.config, input_path);

	// Load the binary input
	char featurized_data_path [AD_PATH_SIZE];
	snprintf(featurized_data_path, AD_PATH_SIZE, "../data/%s", som.config.featurized_data_file);
	FILE* file = fopen(featurized_data_path, "r");
	if (!file) fprintf(stderr, "cannot open input file, path = %s\n", featurized_data_path);
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

	// Initialize the algorithm
	som_init(&som, input_data, rows, cols);

	// Loop: Find each point's winning cluster, and then adjust this cluster and neighboring
	// clusters to be closer to this point. Break when MSE reaches a threshold.
	uint32 i = 0;
	float32 last_error = FLT_MAX;
	while (true) {
		mtx_for(som.inputs, input) {
			uint32 cluster = find_winning_cluster(&som, input);
			calculate_weight_deltas(&som, input, cluster);
			som.winners[mtx_indexof(som.inputs, input)] = cluster;
		}

		apply_deltas(&som);

		// MSE between all inputs and their winning cluster
		float32 error = 0;
		mtx_for(som.inputs, input) {
			uint32 cluster = som.winners[mtx_indexof(som.inputs, input)];
			vector_t weight = mtx_at(som.weights, cluster);
			error += squared_error(weight, input);
		}
		float32 delta_error = abs(error - last_error);
		last_error = error;

		printf("iteration = %d, error = %f\n", i++, error);
		if (delta_error < som.config.error_threshold) {
			break;
		}
	}

	for (uint32 i = 0; i < som.winners.size; i++) {
		printf("input %d: %d\n", i, (uint32)som.winners[i]);
	}
	mtx_for(som.weights, weight) {
		uint32 cluster = mtx_indexof(som.weights, weight);
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
