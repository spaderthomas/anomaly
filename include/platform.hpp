#ifndef AD_PLATFORM_H
#define AD_PLATFORM_H

#include "types.hpp"

namespace paths {
	extern char ad_build              [256];
	extern char ad_configs            [256];
	extern char ad_datas              [256];
	extern char ad_gen_executable     [256];
	extern char ad_feature_executable [256];
	extern char ad_train_executable   [256];
	extern char ad_install            [256];
	extern char ad_project_root       [256];

	void ad_data(const char* data, char* buf, int32 n);
	void ad_config(const char* config, char* buf, int32 n);
};

void init_paths();

#endif
