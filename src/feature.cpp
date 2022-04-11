#include <string.h>
#include <cstdio>

#include "types.hpp"
#include "pack.hpp"

#define AM_PATH_SIZE 256
#define AM_FLAG_OUTPUT "-o"

int main(int arg_count, char** args) {
	char output_path [AM_PATH_SIZE] = { 0 };
	
	for (int32 i = 1; i < arg_count; i++) {
		char* flag = args[i];
		if (!strcmp(flag, AM_FLAG_OUTPUT)) {
			char* arg = args[++i];
			strncpy(output_path, arg, AM_PATH_SIZE);
		}
	}

	printf("%s\n", output_path);
	return 0;
}
