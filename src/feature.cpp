#include <string.h>
#include <stdlib.h>
#include <cstdio>
#include <cassert>
#include <vector>

#include "types.hpp"
#include "pack.hpp"
#include "som.hpp"
#include "platform.hpp"
#include "pipeline.hpp"

#define AD_FLAG_CONFIG "-c"
#define AD_FLAG_HELP "-h"


// Utilities
ad_return_t write_featurized_data(ad_featurized_header* header, std::vector<float32>* buffer, const char* path) {
	FILE* file = fopen(path, "w+");
	if (!file) return AD_RETURN_BAD_FILE;

	fwrite(header, sizeof(ad_featurized_header), 1, file);
	fwrite(buffer->data(), buffer->size() * sizeof(float32), 1, file);
	fclose(file);

	return AD_RETURN_SUCCESS;
}


// Featurize functions. These take different input types and convert them to one or more
// floating point numbers
uint32 ad_featurize_float(std::vector<float32>* buffer, float32* feature) {
	buffer->push_back(*feature);
		return 1;
}

bool is_path_prefix(const char* path, const char* start) {
	size_t ls = strlen(start);
	size_t lp = strlen(path);
	if (ls > lp) return false;

	return !strncmp(start, path, ls - 1);
}

uint32 ad_featurize_path(std::vector<float32>* buffer, char* path) {
	uint32 features = 0;

	float32 is_system_binary = 0;
	if (is_path_prefix(path, "/bin"))     is_system_binary = 1;
	if (is_path_prefix(path, "/usr/bin")) is_system_binary = 1;
	buffer->push_back(is_system_binary);
	features++;

	float32 is_user_dir = 0;
	if (is_path_prefix(path, "/Users")) is_user_dir = 1;
	buffer->push_back(is_user_dir);
	features++;

	return features;
}



// Public API: Featurize some data into a buffer. The data may come from a file (specified in the config),
// or it may come from an in-memory buffer.
void ad_featurize(ad_unpack_context* context, std::vector<float32>* output_buffer, ad_featurized_header* header, bool quiet) {
	uint32 features_written_row = 0; // How many floats have we written for this row? Should all match
	uint32 features_per_row = 0;
	
	ad_feature* feature = nullptr;
	void* data = nullptr;
	while (!unpack_ctx_done(context)) {
		ad_return_t code = unpack_ctx_next(context, &feature, &data);
		if (code) { fprintf(stderr, "unpack error, code = %d\n", code); exit(1); }
		
		if (feature->type == ad_feature_type::ad_row) {
			if (!quiet) printf("row\n");

			header->rows++;
			
			// As soon as we see a row, mark down its feature count so all following rows match.
			// Note that this is not useful on the first row
			if (!features_per_row) header->features_per_row = features_written_row;
			assert(header->features_per_row == features_written_row);
			features_written_row = 0;
		}
		else if (feature->type == ad_feature_type::ad_float) {
			if (!quiet) printf("float: %f\n", *(float*)data);

			uint32 count = ad_featurize_float(output_buffer, (float*)data);
			features_written_row += count;
		}
		else if (feature->type == ad_feature_type::ad_path) {
			if (!quiet) printf("path: %s\n", (char*)data);
			uint32 count = ad_featurize_path(output_buffer, (char*)data);
			features_written_row += count;
		}
	}
	
	if (!quiet) printf("featurizing done, rows = %d, features per row = %d", header->rows, header->features_per_row);
}

void ad_featurize(config_t* config, std::vector<float32>* buffer) {
	static char input_path  [AD_PATH_SIZE];
	static char output_path [AD_PATH_SIZE];
	paths::ad_data(config->raw_data_file,        input_path,  AD_PATH_SIZE);
	paths::ad_data(config->featurized_data_file, output_path, AD_PATH_SIZE);

	ad_unpack_context context;
	if (unpack_ctx_init(&context, input_path)) {
		fprintf(stderr, "could not open raw dataset, path = %s", input_path);
		exit(1);
	}
	
	ad_featurized_header header;
	
	ad_featurize(&context, buffer, &header, config->quiet);
	if (config->write_output) write_featurized_data(&header, buffer, output_path);
}


// CLI
#ifndef AD_GUI
const char* help =
	"ad_featurize: featurize a raw dataset\n\n"
	
	"usage:\n"
	"  -c [config_path]: required, path to config file\n";

int main(int arg_count, char** args) {
	char config_path  [AD_PATH_SIZE] = { 0 };

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

	config_t config;
	cfg_load(&config, config_path);

	std::vector<float32> buffer;
	ad_featurize(&config, &buffer);
	
	return 0;
}
#endif
