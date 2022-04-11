#include <string.h>
#include <stdlib.h>
#include <cstdio>

#include "types.hpp"
#include "pack.hpp"

#define AM_PATH_SIZE 256
#define AM_FLAG_OUTPUT "-o"
#define AM_FLAG_INPUT "-i"
#define AM_FLAG_HELP "-h"

const char* help =
	"ad_featurize: featurize a raw dataset\n"
	"usage:\n"
	"  -i [input_path]: required, path to raw dataset\n"
	"  -o [output_path]: required, path where featurized dataset will be written";
	
int main(int arg_count, char** args) {
	char input_path  [AM_PATH_SIZE] = { 0 };
	char output_path [AM_PATH_SIZE] = { 0 };

	for (int32 i = 1; i < arg_count; i++) {
		char* flag = args[i];
		if (!strcmp(flag, AM_FLAG_OUTPUT)) {
			char* arg = args[++i];
			strncpy(output_path, arg, AM_PATH_SIZE);
		}
		else if (!strcmp(flag, AM_FLAG_INPUT)) {
			char* arg = args[++i];
			strncpy(input_path, arg, AM_PATH_SIZE);
		}
		else if (!strcmp(flag, AM_FLAG_HELP)) {
			printf("%s\n", help);
			exit(0);
		}
	}

	if (!strlen(output_path)) {
		printf("%s\n", help);
		exit(1);
	}

	printf("%s\n", input_path);
	ad_unpack_context context;
	if (!unpack_ctx_init(&context, input_path)) {
		fprintf(stderr, "could not open raw dataset, path = %s", input_path);
		exit(1);
	}
	
	return 0;
}
