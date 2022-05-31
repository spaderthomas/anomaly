#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <float.h>
#ifdef _WIN32
#include <assert.h>
#endif

#include "ini.hpp"

#include "types.hpp"
#include "math.hpp"
#include "som.hpp"

int ini_load_value(void* user, const char* section, const char* name, const char* value) {
    config_t* config = (config_t*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	#define COPY_STRING(s, n) if (MATCH(s, #n)) strncpy(config->n, value, strlen(value))
	#define COPY_U32(s, n) if (MATCH(s, #n)) config->n = atoi(value);
	#define COPY_F32(s, n) if (MATCH(s, #n)) config->n = atof(value);
#define COPY_BOOL(s, n) if (MATCH(s, #n)) config->n = (s[0] == 't' ? true : false);

	COPY_STRING("generator", name);
	COPY_STRING("generator", generator_function);
	COPY_STRING("generator", raw_data_file);
	COPY_STRING("generator", featurized_data_file);
	COPY_STRING("generator", results_file);

	COPY_STRING("som", neighborhood_function);
	COPY_U32   ("som", count_clusters);
	COPY_F32   ("som", learning_rate);
	COPY_F32   ("som", decay_rate);
	COPY_F32   ("som", error_threshold);
	COPY_U32   ("som", seed);

	COPY_BOOL  ("som", quiet);
	COPY_BOOL  ("som", write_output);
    return 1;
}

void cfg_save(config_t* cfg, const char* path) {
	FILE* file = fopen(path, "w+");
	if (!file) return;

	static const char* section_generator = "[generator]\n";
	static const char* section_som = "[som]\n";
	fwrite(section_generator, strlen(section_generator), 1, file);
	fprintf(file, "name = %s\n", cfg->name);
	fprintf(file, "generator_function = %s\n", cfg->generator_function);
	fprintf(file, "raw_data_file = %s\n", cfg->raw_data_file);
	fprintf(file, "featurized_data_file = %s\n", cfg->featurized_data_file);

	fwrite(section_som, strlen(section_som), 1, file);
	fprintf(file, "neighborhood_function = %s\n", cfg->neighborhood_function);
	fprintf(file, "learning_rate = %f\n", cfg->learning_rate);
	fprintf(file, "decay_rate = %f\n", cfg->decay_rate);
	fprintf(file, "count_clusters = %d\n", cfg->count_clusters);
	fprintf(file, "error_threshold = %f\n", cfg->error_threshold);
	fprintf(file, "seed = %d\n", cfg->seed);
	fclose(file);
}

void cfg_load(config_t* config, const char* path) {
    if (ini_parse(path, ini_load_value, config) < 0) {
        printf("cannot load config file, path = %s\n", path);
        return;
    }
}

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

void som_init(som_t* som, float32* input_data, uint32 rows, uint32 cols) {
	if (som->config.seed) srand(som->config.seed);

	mtx_init(&som->inputs, input_data, rows, cols);
	mtx_init(&som->weights, som->config.count_clusters, cols);
	mtx_init(&som->deltas, som->config.count_clusters, cols);
	vec_init(&som->winners, rows);
	arr_init(&som->input_order, rows);

	for (uint32 i = 0; i < rows; i++) arr_push(&som->input_order, i);
	for (uint32 i = 0; i < rows; i++) {
		float32 j = rand() % rows;
		uint32* vi = som->input_order[i];
		uint32* vj = som->input_order[j];
		uint32  vt = *vi;

		*vi = *vj;
		*vj = vt;
	}

	mtx_for(som->inputs, input) {
		vec_normalize(input);
	}

	mtx_for(som->weights, weight) {
		vec_for(weight, w) {
			*w = rand_float32(1);
		}
	}
	mtx_for(som->weights, weight) {
		vec_normalize(weight);
	}
}

void som_iterate(som_t* som) {
	som->iteration++;

	arr_for(som->input_order, i) {
		vector_t input = mtx_at(som->inputs, *i);
		uint32 cluster = find_winning_cluster(som, input);
		calculate_weight_deltas(som, input, cluster);
		som->winners[*i] = cluster;
	}
}

float32 som_error(som_t* som) {
	float32 error = 0;
	mtx_for(som->inputs, input) {
		uint32 cluster = som->winners[mtx_indexof(som->inputs, input)];
		vector_t weight = mtx_at(som->weights, cluster);
		error += squared_error(weight, input);
	}
	return error;
}

uint32 find_winning_cluster(som_t* som, vector_t& input) {
	uint32 winning_cluster = 0;
	float32 min_distance = FLT_MAX;
	
	mtx_for(som->weights, weight) {
		uint32 cluster = mtx_indexof(som->weights, weight);
		float32 distance = vec_distance(input, weight);
		if (min_distance > distance) {
			winning_cluster = cluster;
			min_distance = distance;
		}
	}

	return winning_cluster;
}

float32 decayed_learning_rate(som_t* som) {
	float32 decayed = som->config.learning_rate * (1 - som->iteration / som->config.decay_rate);
	return fmax(decayed, 0);
}

void calculate_weight_deltas(som_t* som, vector_t& input, uint32 winning_cluster) {
	mtx_for(som->weights, weight) {
		uint32 cluster = mtx_indexof(som->weights, weight);
		float32 strength = ns_linear(winning_cluster, cluster);
		for (int i = 0; i < input.size; i++) {
			*mtx_at(som->deltas, cluster, i) += decayed_learning_rate(som) * strength * (input[i] - weight[i]);
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

void apply_deltas(som_t* som) {
	mtx_scale(som->deltas, 1.f / mtx_size(som->deltas));
	mtx_add(som->weights, som->deltas);
	memset(som->deltas.data, 0, sizeof(float32) * mtx_size(som->deltas));
}
