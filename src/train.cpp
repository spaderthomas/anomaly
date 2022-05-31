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
#include "array.hpp"
#include "som.hpp"
#include "platform.hpp"
#include "pipeline.hpp"

#define AD_FLAG_CONFIG "-c"
#define AD_FLAG_HELP "-h"

void ad_train(som_t& som, ad_featurized_header* header, float32* input_data) {
	// Initialize the algorithm
	uint32 rows = header->rows;
	uint32 cols = header->features_per_row;
	som_init(&som, input_data, rows, cols);
	
	// Loop: Find each point's winning cluster, and then adjust this cluster and neighboring
	// clusters to be closer to this point. Break when MSE reaches a threshold.
	uint32 i = 0;
	float32 last_error = FLT_MAX;
	while (true) {
		som_iterate(&som);
		apply_deltas(&som);

		float32 error = som_error(&som);
		float32 delta_error = abs(error - last_error);
		last_error = error;

		if (!som.config.quiet) {
			printf("iteration = %d, error = %f, learning_rate = %f\n", i++, error, decayed_learning_rate(&som));
		}
		
		if (delta_error < som.config.error_threshold) {
			break;
		}
	}

	if (!som.config.quiet) {
		for (uint32 i = 0; i < som.winners.size; i++) {
			printf("input %d: %d\n", i, (uint32)som.winners[i]);
		}
		mtx_for(som.weights, weight) {
			uint32 cluster = mtx_indexof(som.weights, weight);
			printf("cluster %d: (%f, %f, %f, %f)\n", cluster, weight[0], weight[1], weight[2], weight[3]);
		}
		
		printf("done\n");
	}
}

void ad_train(som_t& som) {
	// Load the binary input
	char featurized_data_path [AD_PATH_SIZE];
	paths::ad_data(som.config.featurized_data_file, featurized_data_path, AD_PATH_SIZE);
				   
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
	float32* input_data = (float32*)(buffer + sizeof(ad_featurized_header));

	ad_train(som, header, input_data);
}

#ifndef AD_GUI
const char* help =
	"ad_train: train a model on a featurized dataset\n\n"
	
	"usage:\n"
	"  -c [config_path]: required, path to a config file";

int main(int arg_count, char** args) {
	char config_path [AD_PATH_SIZE] = { 0 };

	// Parse arguments and check for validity
	for (int32 i = 1; i < arg_count; i++) {
		char* flag = args[i];
		if (!strcmp(flag, AD_FLAG_CONFIG)) {
			char* arg = args[++i];
			strncpy(config_path, arg, AD_PATH_SIZE);
		}
		else if (!strcmp(flag, AD_FLAG_HELP)) {
			printf("%s\n", help);
			exit(0);
		}
	}

	if (!strlen(config_path)) {
		printf("%s\n", help);
		exit(1);
	}

	init_paths();
	
	som_t som;
	cfg_load(&som.config, config_path);
	
	ad_train(som);
}
#endif
