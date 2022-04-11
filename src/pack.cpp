#include <cassert>
#include <memory>
#include "pack.hpp"

// entry_magic is used to sanity-check the headers for a feature
// dataset_magic is one-per-dataset, and can be used to check that the versions between
// dataset matches between what the loading-program was compiled with and what the
// generating-program was compiled with
uint8  ad_feature::entry_magic   = 0x46;
uint32 ad_feature::dataset_magic = 0x00000001; 



void pack_ctx_data(ad_pack_context* context, const void* data, int32 data_size) {
	// You need room for this data + the header that ends the dataset
	assert(context->bytes_written + data_size + sizeof(ad_feature) <= context->buffer_size);
	
	memcpy(context->buffer + context->bytes_written, data, data_size);
	context->bytes_written += data_size;
}

void pack_ctx_init(ad_pack_context* context, char* buffer, int32 buffer_size) {
	assert(buffer);
	assert(buffer_size >= sizeof(int32)); // At least the magic header
	
	context->buffer = buffer;
	context->buffer_size = buffer_size;
	context->bytes_written = 0;

	pack_ctx_data(context, &ad_feature::dataset_magic, sizeof(int32));
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
	feature.size = data_size + 1;
	pack_ctx_header(context, feature);

	pack_ctx_data(context, data, data_size);
	pack_ctx_data(context, &null_terminator, sizeof(char));
}

ad_return_t pack_ctx_write(ad_pack_context* context, const char* path) {
	FILE* file = fopen(path, "w+");
	if (!file) return AD_RETURN_BAD_FILE;

	fwrite(context->buffer, context->buffer_size, 1, file);
	fclose(file);

	return AD_RETURN_SUCCESS;
}

bool validate_header(ad_feature* header) {
	return header->magic == ad_feature::entry_magic;
}

ad_return_t unpack_ctx_init(ad_unpack_context* context, const char* filepath) {
	FILE* file = fopen(filepath, "r");
	if (!file) return AD_RETURN_BAD_FILE;

	fseek(file, 0, SEEK_END);
	context->buffer_size = ftell(file);
	assert(context->buffer_size >= sizeof(int32)); // Must at least include magic header
	fseek(file, 0, SEEK_SET);

	context->buffer = (char*)calloc(sizeof(char), context->buffer_size);
	fread(context->buffer, context->buffer_size, 1, file);
	fclose(file);

	context->bytes_read = 0;

	uint32* magic = (uint32*)context->buffer;
	if (*magic != ad_feature::dataset_magic) {
		fprintf(stderr, "mismatch between dataset header and expected header, actual = %d, expected = %d\n",
				*magic, ad_feature::dataset_magic);
		return AD_RETURN_BAD_HEADER;
	}
	context->bytes_read += sizeof(int32);
	
	return AD_RETURN_SUCCESS;
};

ad_return_t unpack_ctx_done(ad_unpack_context* context) {
	if (context->bytes_read >= context->buffer_size) return AD_RETURN_NOT_DONE;

	ad_feature* header = (ad_feature*)(context->buffer + context->bytes_read);
	if (!validate_header(header)) return AD_RETURN_BAD_HEADER;

	return header->type == ad_feature_type::ad_end ? AD_RETURN_DONE : AD_RETURN_NOT_DONE;
}

ad_return_t unpack_ctx_next(ad_unpack_context* context, ad_feature** header, void** data) {
	*header = (ad_feature*)(context->buffer + context->bytes_read);
	if (!validate_header(*header)) return AD_RETURN_BAD_HEADER;

	context->bytes_read += sizeof(ad_feature);
	*data = context->buffer + context->bytes_read;
	context->bytes_read += (*header)->size;
	
	return AD_RETURN_SUCCESS;
}
