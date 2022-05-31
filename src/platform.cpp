#include <cstdio>

#include "platform.hpp"

// Build up all paths in the project from the project root. Each path has a corresponding
// buffer that holds the path at runtime. The buffer is filled in my snprintf'ing the
// project root into these format strings
#define _ad_project_root "%s"
#define _ad_build              _ad_project_root "/" "build"
#define _ad_datas              _ad_project_root "/" "data"
#define _ad_data               _ad_datas        "/" "%%s"
#define _ad_configs            _ad_datas        "/" "config"
#define _ad_config             _ad_configs      "/" "%%s.ini"
#define _ad_gen_executable     _ad_install      "/" "ad_gen"       _ad_executable_ext
#define _ad_feature_executable _ad_install      "/" "ad_featurize" _ad_executable_ext
#define _ad_train_executable   _ad_install      "/" "ad_train"     _ad_executable_ext
#define _ad_install            _ad_build        "/" "bin"

#ifdef _WIN32
#include <windows.h>

#define _ad_executable_ext ".exe"

namespace paths {
	// Runtime buffers to hold paths
	char ad_project_root       [256];
	char ad_build              [256];
	char ad_configs            [256];
	char ad_datas              [256];
	char ad_feature_executable [256];
	char ad_gen_executable     [256];
	char ad_train_executable   [256];
	char ad_install            [256];

	// Except some runtime paths are themselves format strings (e.g. to get
	// a config, we want /path/to/configs/%s.ini
	char __ad_data             [256];
	char __ad_config           [256];
	void ad_data(const char* data, char* buf, int32 n) { snprintf(buf, n, __ad_data, data); }
	void ad_config(const char* config, char* buf, int32 n) { snprintf(buf, n, __ad_config, config); }
};

void init_paths() {
	// Get the project root
	GetModuleFileNameA(NULL, paths::ad_project_root, 256);
	uint32 len = (uint32)strlen(paths::ad_project_root);

	// Normalize
	for (uint32 i = 0; i < len; i++) {
		if (paths::ad_project_root[i] == '\\') paths::ad_project_root[i] = '/';
	}

	// Remove executable name, 'bin', and 'build'
	int32 removed = 0;
	for (uint32 i = len - 1; i > 0; i--) {
		if (paths::ad_project_root[i] == '/') removed++;
		paths::ad_project_root[i] = 0;
		if (removed == 3) break;
	}

	// Build the other paths from the project root
	snprintf(paths::ad_build,              256, _ad_build,              paths::ad_project_root);
	snprintf(paths::ad_configs,            256, _ad_configs,            paths::ad_project_root);
	snprintf(paths::__ad_config,           256, _ad_config,             paths::ad_project_root);
	snprintf(paths::ad_datas,              256, _ad_datas,              paths::ad_project_root);
	snprintf(paths::__ad_data,             256, _ad_data,               paths::ad_project_root);
	snprintf(paths::ad_install,            256, _ad_install,            paths::ad_project_root);
	snprintf(paths::ad_gen_executable,     256, _ad_gen_executable,     paths::ad_project_root);
	snprintf(paths::ad_feature_executable, 256, _ad_feature_executable, paths::ad_project_root);
	snprintf(paths::ad_train_executable,   256, _ad_train_executable,   paths::ad_project_root);
}

#endif
