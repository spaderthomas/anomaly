#include <cassert>
#include <memory>
#include "pack.hpp"

void pack_ctx_init(ad_pack_context* context, char* buffer, int32 buffer_size) {
	assert(buffer);
	context->buffer = buffer;
	context->buffer_size = buffer_size;
	context->bytes_written = 0;
}

void pack_ctx_data(ad_pack_context* context, const void* data, int32 data_size) {
	// You need room for this data + the header that ends the dataset
	assert(context->bytes_written + data_size + sizeof(ad_feature) <= context->buffer_size);
	
	memcpy(context->buffer + context->bytes_written, data, data_size);
	context->bytes_written += data_size;
}

void pack_ctx_header(ad_pack_context* context, ad_feature feature) {
	context->last_feature = context->buffer + context->bytes_written;
	pack_ctx_data(context, &feature, sizeof(ad_feature));
}

void pack_ctx_row(ad_pack_context* context) {
	ad_feature feature;
	feature.type = ad_feature_type::ad_row;
	pack_ctx_header(context, feature);
}

void pack_ctx_end(ad_pack_context* context) {
	ad_feature feature;
	feature.type = ad_feature_type::ad_end;
	pack_ctx_header(context, feature);
}

void pack_ctx_float32(ad_pack_context* context, float32 data) {
	ad_feature feature;
	feature.type = ad_feature_type::ad_float;
	feature.size = sizeof(float32);
	pack_ctx_header(context, feature);

	pack_ctx_data(context, &data, sizeof(float32));
}

void pack_ctx_string(ad_pack_context* context, const char* data, int32 data_size) {
	char null_terminator = 0;
	
	ad_feature feature;
	feature.type = ad_feature_type::ad_string;
	feature.size = data_size;
	pack_ctx_header(context, feature);

	pack_ctx_data(context, data, data_size);
	pack_ctx_data(context, &null_terminator, sizeof(char));
}

bool unpack_ctx_init(ad_unpack_context* context, const char* filepath) {
	FILE* file = fopen(filepath, "r");
	if (!file) return false;

	fseek(file, 0, SEEK_END);
	context->buffer_size = ftell(file);
	context->buffer = (char*)calloc(sizeof(char), context->buffer_size);
	fread(context->buffer, context->buffer_size, 1, file);

	context->bytes_read = 0;
	return true;
};
