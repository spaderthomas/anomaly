#ifndef AD_SOM_H
#define AD_SOM_H

#include "math.hpp"
#include "array.hpp"

struct config_t {
	char name                  [64] = {0};
	char generator_function    [64] = {0};
	char raw_data_file        [256] = {0};
	char featurized_data_file [256] = {0};
	char results_file         [256] = {0};

	char neighborhood_function [256] = {0};
	float32 learning_rate            =  0;
	float32 decay_rate               =  0;
	uint32 count_clusters            =  0;
	float32 error_threshold          =  0;
	uint32 seed                      =  0;

	bool quiet        = false;
	bool write_output = false;

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
	array_t<uint32> input_order;
	uint32 iteration = 0;
};

void som_init(som_t* som, float32* input_data, uint32 rows, uint32 cols);
void som_iterate(som_t* som);
float32 som_error(som_t* som);
float32 decayed_learning_rate(som_t* som);
uint32 find_winning_cluster(som_t* som, vector_t& input);
void calculate_weight_deltas(som_t* som, vector_t& input, uint32 winning_cluster);
float32 squared_error(vector_t& weight, vector_t& input);
void apply_deltas(som_t* som);

#endif
