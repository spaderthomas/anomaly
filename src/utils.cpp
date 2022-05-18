#include <memory>

#include "array.hpp"

void memfill(void* dst, int32 size, void* pattern, int32 pattern_size) {
	char* cdst = (char*)dst;
	char* cptn = (char*)pattern;
	int i = 0;

	while (true) {
		if (i + pattern_size > size) return;
		memcpy(cdst + i, cptn, pattern_size);
		i += pattern_size;
	}
}
