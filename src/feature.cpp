const char* help =
	"ad_featurize: featurize a raw dataset\n\n"
	
	"usage:\n"
	"  -i [input_path]: required, path to raw dataset\n"
	"  -o [output_path]: required, path where featurized dataset will be written";

#include <string.h>
#include <stdlib.h>
#include <cstdio>
#include <cassert>
#include <vector>

#include "types.hpp"
#include "pack.hpp"

#define AD_FLAG_OUTPUT "-o"
#define AD_FLAG_INPUT "-i"
#define AD_FLAG_HELP "-h"
#define AD_FLAG_LOG "-l"

bool do_log = false;

ad_return_t write_featurized_data(ad_featurized_header* header, std::vector<float32>* buffer, const char* path) {
	FILE* file = fopen(path, "w+");
	if (!file) return AD_RETURN_BAD_FILE;

	fwrite(header, sizeof(ad_featurized_header), 1, file);
	fwrite(buffer->data(), buffer->size() * sizeof(float32), 1, file);
	fclose(file);

	return AD_RETURN_SUCCESS;
}


uint32 ad_featurize_float(std::vector<float32>* buffer, float32* feature) {
	buffer->push_back(*feature);
	return 1;
}
	
int main(int arg_count, char** args) {
	std::vector<float32> featurized;
	char input_path  [AD_PATH_SIZE] = { 0 };
	char output_path [AD_PATH_SIZE] = { 0 };

	for (int32 i = 1; i < arg_count; i++) {
		char* flag = args[i];
		if (!strcmp(flag, AD_FLAG_OUTPUT)) {
			char* arg = args[++i];
			strncpy(output_path, arg, AD_PATH_SIZE);
		}
		else if (!strcmp(flag, AD_FLAG_INPUT)) {
			char* arg = args[++i];
			strncpy(input_path, arg, AD_PATH_SIZE);
		}
		else if (!strcmp(flag, AD_FLAG_LOG)) {
			do_log = true;
		}
		else if (!strcmp(flag, AD_FLAG_HELP)) {
			printf("%s\n", help);
			exit(0);
		}
	}

	if (!strlen(output_path)) {
		printf("%s\n", help);
		exit(1);
	}

	ad_unpack_context context;
	if (unpack_ctx_init(&context, input_path)) {
		fprintf(stderr, "could not open raw dataset, path = %s", input_path);
		exit(1);
	}


	ad_featurized_header info;
	uint32 features_written_row = 0; // How many floats have we written for this row? Should all match
	uint32 features_per_row = 0;
	
	ad_feature* header = nullptr;
	void* data = nullptr;
	while (!unpack_ctx_done(&context)) {
		ad_return_t code = unpack_ctx_next(&context, &header, &data);
		if (code) { fprintf(stderr, "unpack error, code = %d\n", code); exit(1); }

		
		if (header->type == ad_feature_type::ad_row) {
			if (do_log) printf("row\n");

			info.rows++;
			
			// As soon as we see a row, mark down its feature count so all following rows match.
			// Note that this is not useful on the first row
			if (!features_per_row) info.features_per_row = features_written_row;
			assert(info.features_per_row == features_written_row);
			features_written_row = 0;
		}
		else if (header->type == ad_feature_type::ad_float) {
			if (do_log) printf("float: %f\n", *(float*)data);

			uint32 count = ad_featurize_float(&featurized, (float*)data);
			features_written_row += count;
		}
	}

	if (do_log) printf("featurizing done, rows = %d, features per row = %d", info.rows, info.features_per_row);
	write_featurized_data(&info, &featurized, output_path);
	return 0;
}
