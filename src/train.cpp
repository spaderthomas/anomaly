const char* help =
	"ad_train: train a model on a featurized dataset\n\n"
	
	"usage:\n"
	"  -i [input_path]: required, path to a featurized dataset";

#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <span>

#include "types.hpp"
#include "pack.hpp"

#define AD_FLAG_INPUT "-i"
#define AD_FLAG_HELP "-h"

int main(int arg_count, char** args) {
	char input_path [AD_PATH_SIZE] = { 0 };

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
	uint32 data_size = header->rows * header->features_per_row;
	std::span<float32> data((float32*)(buffer + sizeof(ad_featurized_header)), data_size);
}
