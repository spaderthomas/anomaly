#ifndef AD_SOM_H
#define AD_SOM_H

#include "math.hpp"

struct config_t {
	char name                  [64];
	char generator_function    [64];
	char raw_data_file        [256];
	char featurized_data_file [256];

	char neighborhood_function [256];
	float32 learning_rate;
	uint32 count_clusters;
	float32 error_threshold;
	uint32 seed;
};
void cfg_save(config_t* cfg, const char* path);
void cfg_load(config_t* config, const char* path);


typedef float32 (*ns_function)(uint32, uint32);
float32 ns_linear(uint32 winning_cluster, uint32 neighbor_cluster);
float32 ns_none(uint32 winning_cluster, uint32 neighbor_cluster);

struct som_t {
    config_t config;
    matrix_t weights;
    matrix_t inputs;
    matrix_t deltas;
    vector_t winners;
};

void som_init(som_t* som, float32* input_data, uint32 rows, uint32 cols);
uint32 find_winning_cluster(som_t* som, vector_t& input);
void calculate_weight_deltas(som_t* som, vector_t& input, uint32 winning_cluster);
float32 squared_error(vector_t& weight, vector_t& input);
void apply_deltas(som_t* som);


#endif