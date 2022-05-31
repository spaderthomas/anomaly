#ifndef AD_TRAIN_H
#define AD_TRAIN_H

#include "pack.hpp"
#include "som.hpp"
#include <vector>

void ad_generate(config_t* config, char* buffer, uint32 buffer_size);
void ad_featurize(config_t* config, std::vector<float32>* buffer);
void ad_featurize(ad_unpack_context* context, std::vector<float32>* buffer, ad_featurized_header* header, bool quiet = true);
void ad_train(som_t& som, ad_featurized_header* header, float32* input_data);
void ad_train(som_t& som);

#endif
