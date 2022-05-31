#include <memory>
#include <cassert>
#include <cstring>

#include "pack.hpp"
#include "platform.hpp"
#include "pipeline.hpp"
#include "som.hpp"

#define AD_FLAG_CONFIG "-c"
#define AD_FLAG_HELP "-h"

typedef void (*gen_fn)(config_t*, char*, uint32);

// Utility
bool is_numeral(char c) {
	return c >= '0' && c <= '9';
}

bool is_comma(char c) {
	return c == ',';
}

bool is_newline(char c) {
	return c == '\n';
}

int32 ctoi(char c) {
	assert(is_numeral(c));
	return c - '0';
}

// Generator functions
void gen_test(config_t* config, char* buffer, uint32 buffer_size) {
	printf("generating data using gen_test\n");
	static char output_path [AD_PATH_SIZE];
	paths::ad_data(config->raw_data_file, output_path, AD_PATH_SIZE);
	
	// Simple 2D points that should map to two linearly separable clusters
	float data_set [8][2] = {
		{ 1.f, 1.f },
		{ .9f, .9f },
		{ .8f, 1.f },
		{ 1.f, .75f },

		{ -1.f, -1.f },
		{ -.9f, -.9f },
		{ -.75f, -.9f },
		{ -.9f, -.75f },
	};
	
	ad_pack_context context;
	pack_ctx_init(&context, buffer, buffer_size);

	for (int i = 0; i < 8; i++) {
		pack_ctx_row(&context);
		pack_ctx_float32(&context, data_set[i][0]);
		pack_ctx_float32(&context, data_set[i][1]);
	}
	pack_ctx_end(&context);

	pack_ctx_write(&context, output_path);
}

void gen_iris(config_t* config, char* buffer, uint32 buffer_size) {
	printf("generating data using gen_iris\n");
	static char output_path [AD_PATH_SIZE];
	paths::ad_data(config->raw_data_file, output_path, AD_PATH_SIZE);
	
	ad_pack_context context;
	pack_ctx_init(&context, buffer, buffer_size);

	// Load the file data
	static char path [256];
	paths::ad_data("iris.csv", path, 256);
	
	FILE* file = fopen(path, "r");
	if (!file) {
		fprintf(stderr, "failed to load iris dataset, path = %s", path);
		exit(1);
	}
			
	fseek(file, 0, SEEK_END);
	uint32 file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* file_data = (char*)calloc(sizeof(char), file_size);
	fread(file_data, file_size, 1, file);
	fclose(file);

	pack_ctx_row(&context); // Start the first row, couldn't figure out how to roll this into loop

	for(uint32 i = 0; i < file_size;) {
		char c = file_data[i];
		// Parse a float, which are all of the format X.Y
		if (is_numeral(c)) {
			float32 value = (float32)ctoi(file_data[i++]);
			i++;
			char decimal = file_data[i++];
			value = value + (ctoi(decimal) / 10.f);
			pack_ctx_float32(&context, value);
		}
		// Advance to the next entry
		else if (is_comma(c)) {
			i++;
		}
		// Skip the flower label
		else {
    		while (!is_newline(c) && i < file_size) c = file_data[i++];
            if (i == file_size) break;
			pack_ctx_row(&context);
		}
	}

	pack_ctx_end(&context);
	pack_ctx_write(&context, output_path);
}

void gen_paths(config_t* config, char* buffer, uint32 buffer_size) {
	printf("generating data using gen_path\n");
	static char output_path [AD_PATH_SIZE];
	paths::ad_data(config->raw_data_file, output_path, AD_PATH_SIZE);
	
	// Simple 2D points that should map to two linearly separable clusters
	const char* data_set [] = {
        "/usr/bin/ssh",
        "/usr/bin/grep",
        "/bin/mv",
        "/bin/zsh",

        "/Users/tspader/source/anomaly/build/ad_gen",
        "/Users/tspader/source/anomaly/build/ad_featurize",
        "/Users/tspader/source/anomaly/build/ad_train",
        "/Users/tspader/source/anomaly/build/ad_gui",
	};
	
	ad_pack_context context;
	pack_ctx_init(&context, buffer, buffer_size);

	for (int i = 0; i < 8; i++) {
		pack_ctx_row(&context);
		pack_ctx_path(&context, data_set[i]);
	}
	pack_ctx_end(&context);

	pack_ctx_write(&context, output_path);
}

gen_fn get_generator(const char* generator_name) {
	if (!strcmp(generator_name, "test")) return &gen_test;
	if (!strcmp(generator_name, "iris")) return &gen_iris;
	if (!strcmp(generator_name, "path")) return &gen_paths;

	return nullptr;
}

void ad_generate(config_t* config, char* buffer, uint32 buffer_size) {
	gen_fn generate = get_generator(config->generator_function);
	generate(config, buffer, buffer_size);
}

#ifndef AD_GUI
const char* help =
	"gen.cpp: raw data generation for anomaly detection\n\n"

	"usage:\n"
	"  -o [output_path]: required, file to write output to\n"
	"  -f [function] {test, iris}: function used to generate data";

int main(int arg_count, char** args) {
	static char config_path [AD_PATH_SIZE];
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
	
	// Sanity checks
	if (!strlen(config_path)) {
		printf("%s\n", help);
		exit(1);
	}

	init_paths();

	// Set up a config from the path passed in via command line
	config_t config;
	cfg_load(&config, config_path);

	// Allocate a buffer
	uint32 buffer_size = 1024 * 1024;
	char* buffer = (char*)calloc(sizeof(char), buffer_size);

	// Generate raw data to the buffer
	ad_generate(&config, buffer, buffer_size);
	
	exit(0);
}
#endif
