#ifndef AD_PACK_H
#define AD_PACK_H

#include "types.hpp"

enum class ad_feature_type : int8 {
	ad_row, // Marks the beginning of a row
	ad_end, // Marks the end of the dataset
	ad_float,
	ad_string,
};


struct ad_feature {
	static uint8 entry_magic;
	static uint32 dataset_magic;
	
	int8 magic = ad_feature::entry_magic;
	int16 size = 0;
	ad_feature_type type;
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
ad_return_t pack_ctx_write(ad_pack_context* context, const char* path);

struct ad_unpack_context {
	char* buffer;
	int32 buffer_size;
	int32 bytes_read;

	char* last_feature;	// For debugging
};

ad_return_t unpack_ctx_init(ad_unpack_context* context, const char* filepath);
ad_return_t unpack_ctx_next(ad_unpack_context* context, ad_feature** header, void** data);
ad_return_t unpack_ctx_done(ad_unpack_context* context);

#endif // AD_PACK_H
