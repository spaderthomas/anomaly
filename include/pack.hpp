#include "types.hpp"

enum class ad_feature_type : int16 {
	ad_row, // Marks the beginning of a row
	ad_end, // Marks the end of the dataset
	ad_float,
	ad_string,
};

struct ad_feature {
	ad_feature_type type;
	int16 size;
};

struct ad_pack_context {
	char* buffer;
	int32 buffer_size;
	int32 bytes_written;

	char* last_feature;	// For debugging
};

void pack_ctx_init(ad_pack_context* context, char* buffer, int32 buffer_size);
void pack_ctx_row(ad_pack_context* context);
void pack_ctx_end(ad_pack_context* context);
void pack_ctx_string(ad_pack_context* context, const char* data, int32 data_size);
void pack_ctx_float32(ad_pack_context* context, float32 f);

struct ad_unpack_context {
	char* buffer;
	int32 buffer_size;
	int32 bytes_written;

	char* last_feature;	// For debugging
};
