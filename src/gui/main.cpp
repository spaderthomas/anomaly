#include <iostream>

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imfilebrowser.h"

#include "gui/ogl.hpp"
#include "types.hpp"
#include "som.hpp"
#include "platform.hpp"
#include "pipeline.hpp"

namespace global {
	::ImGui::FileBrowser file_browser;
	bool is_loading_config = false;
	bool show_imgui_demo = false;
	char install_dir [256] = {0};
	uint32 buffer_size = 1024 * 1024;
	char* buffer = (char*)calloc(sizeof(char), buffer_size);
};

struct modals_t {
	bool save_config = false;
};
struct anomaly_t {
	modals_t modals;
	config_t config;

	char* input_data = nullptr;
	uint32 input_data_size;

	// A header describing the featurized data. This is filled in when we make the call
	// to featurize the input data
	ad_featurized_header feature_header;
	std::vector<float32> feature_data;

	// A list of clusters. Order is preserved between this array and the input datapoints.
	vector_t results = { 0 };
	bool has_results = false;
};

void init_memory(anomaly_t* anomaly) {
	anomaly->has_results = false;
	
	if (anomaly->input_data) free(anomaly->input_data);
	anomaly->input_data = nullptr;
	anomaly->input_data_size = 0;
	
	memset(&anomaly->feature_header, 0, sizeof(ad_featurized_header));
	if (anomaly->results.data) vec_free(anomaly->results);

	// @spader: This is an arbitrary number. Any reasonable amount of data will overrun this.
	// Pick a larger number.
	anomaly->input_data_size = 1024 * 1024;
	anomaly->input_data = (char*)calloc(sizeof(char), anomaly->input_data_size);

	anomaly->feature_data = std::vector<float32>();
	
	// @spader: You can't initialize the result buffer, because you do not know how many datapoints
	// there are. Do so after the algorithm has run.
}

void draw_main_menu(anomaly_t* anomaly) {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::BeginMenu("Configuration")) {
				if (ImGui::MenuItem("Load")) {
					global::file_browser.Open();
					global::is_loading_config = true;
				}
				if (ImGui::MenuItem("Save")) {
					anomaly->modals.save_config = true;
				}

				ImGui::EndMenu();
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Window")) {
			ImGui::MenuItem("Show ImGui Demo", "", &global::show_imgui_demo);
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void update_modals(anomaly_t* anomaly) {
	// Modal dialogue to open a popup to prompt the user for a name for this config
	if (anomaly->modals.save_config) {
		ImGui::OpenPopup("Save Configuration");
	}
	if (ImGui::BeginPopupModal("Save Configuration")) {
		ImGui::Text("Name");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(200);
		ImGui::InputText("##main_menu:save_config:name", anomaly->config.name, 256);

		if (ImGui::Button("Save")) {
			if (!strlen(anomaly->config.name)) {
				std::cout << "Cannot save configuration without a name." << std::endl;
				ImGui::OpenPopup("Error");
			} else {
				char path [256];
				snprintf(path, 256, "%s/%s/%s.ini", "..", "data", anomaly->config.name);
				cfg_save(&anomaly->config, path);
				ImGui::CloseCurrentPopup();
			}
		}
			
		ImGui::SameLine();
			
		if (ImGui::Button("Cancel")) {
			ImGui::CloseCurrentPopup();
		}

		if (ImGui::BeginPopup("Error")) {
			ImGui::Text("Cannot save configuration without name.");
			ImGui::EndPopup();
		}

		ImGui::EndPopup();
	}
}

void check_cfg_load(anomaly_t* anomaly) {
	if (!global::is_loading_config) return;
	if (!global::file_browser.HasSelected()) return;
	
	auto path = global::file_browser.GetSelected().string();
	cfg_load(&anomaly->config, path.c_str());
	global::file_browser.ClearSelected();
	global::is_loading_config = false;
}

void draw_cfg_editor(anomaly_t* anomaly) {
	ImGui::Text("ad_gen config");

	int input_size = 200;
	ImGui::Text("function");
	ImGui::SameLine();
	ImGui::SameLine(ImGui::GetWindowWidth() - input_size);
	ImGui::InputText("##ad_gen:function", anomaly->config.generator_function, 64);
		
	ImGui::Text("raw data file");
	ImGui::SameLine();
	ImGui::SameLine(ImGui::GetWindowWidth() - input_size);
	ImGui::InputText("##ad_gen:raw_data_file", anomaly->config.raw_data_file, 256);

	ImGui::Text("featurized data file");
	ImGui::SameLine();
	ImGui::SameLine(ImGui::GetWindowWidth() - input_size);
	ImGui::InputText("##ad_gen:featurized_data_file", anomaly->config.featurized_data_file, 256);

	// Display the config fields for SOM itself
	ImGui::Separator();
	ImGui::Text("som config");

	ImGui::Text("neighborhood function");
	ImGui::SameLine(ImGui::GetWindowWidth() - input_size);
	ImGui::InputText("##som:neighborhood", anomaly->config.neighborhood_function, 256);

	ImGui::Text("learning rate");
	ImGui::SameLine(ImGui::GetWindowWidth() - input_size);
	ImGui::InputFloat("##som:learning_rate", &anomaly->config.learning_rate);

	ImGui::Text("# clusters");
	ImGui::SameLine(ImGui::GetWindowWidth() - input_size);
	ImGui::InputScalar("##som:clusters", ImGuiDataType_U32, &anomaly->config.count_clusters);

	ImGui::Text("error threshold");
	ImGui::SameLine();
	ImGui::SameLine(ImGui::GetWindowWidth() - input_size);
	ImGui::InputFloat("##som:error", &anomaly->config.error_threshold);

	ImGui::Text("seed");
	ImGui::SameLine();
	ImGui::SameLine(ImGui::GetWindowWidth() - input_size);
	ImGui::InputScalar("##som:seed", ImGuiDataType_U32, &anomaly->config.seed);
}

int32 count_input_features(anomaly_t* anomaly) {
	ad_unpack_context unpack;
	unpack_ctx_init(&unpack, anomaly->input_data, anomaly->input_data_size);
	
	ad_feature* feature = nullptr;
	void* data = nullptr;

	bool found_row = false;
	int32 count_features = 0;
	while (!unpack_ctx_done(&unpack)) {
		unpack_ctx_next(&unpack, &feature, &data);
		
		if (feature->type == ad_feature_type::ad_row) {
			if (found_row) break;
			found_row = true;
		} else {
			count_features++;
		}
	}
	
	return count_features;
}

ImVec4 cluster_colors [16] = {
	{ .4f, .9f,  .5f, 1.f },
	{ .4f, .6f,  .9f, 1.f },
	{ .5f, .35f, .8f, 1.f },
	{ .9f, .4f,  .4f, 1.f },
};


int main (int argc, char** argv) {
	anomaly_t anomaly;
	
	init_paths();
  	init_glfw();
  	init_gl();
  	init_imgui();
  	init_memory(&anomaly);

  	while(!glfwWindowShouldClose(window)) {
		double frame_start_time = glfwGetTime();
		double dt = 1.f / 30.f;

		ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

		if (global::show_imgui_demo) ImGui::ShowDemoWindow(&global::show_imgui_demo);

		ImGui::SetNextWindowSize({400, 0});
		ImGui::Begin("anomaly");
		
		draw_main_menu(&anomaly);
		update_modals(&anomaly);
		check_cfg_load(&anomaly);
		draw_cfg_editor(&anomaly);

		// Display the config fields for the generation and featurization
		if (ImGui::Button("Train")) {
			init_memory(&anomaly);
			
			// Generate
			ad_generate(&anomaly.config, anomaly.input_data, anomaly.input_data_size);

			// Featurize
			ad_unpack_context unpack;
			unpack_ctx_init(&unpack, anomaly.input_data, anomaly.input_data_size);
			ad_featurize(&unpack, &anomaly.feature_data, &anomaly.feature_header);

			// Train
			som_t som;
			som.config = anomaly.config;
			ad_train(som, &anomaly.feature_header, anomaly.feature_data.data());

			vec_init(&anomaly.results, som.winners.size);
			memcpy(anomaly.results.data, som.winners.data, som.winners.size * sizeof(float32));
			anomaly.has_results = true;
		}

		// If we have results, display them in a table
		if (anomaly.has_results) {
			ImGui::Begin("Results");
			// The table has two things:
			// 1. The raw inputs
			// 2. The clusters
			ImGuiTableFlags table_flags = 0;
			table_flags |= ImGuiTableFlags_RowBg;
			table_flags |= ImGuiTableFlags_Borders;
			int32 row = -1;
			int32 cols = count_input_features(&anomaly) + 1; // Add a column for the cluster ID
			int32 this_row_cluster = 0;
			
			if (ImGui::BeginTable("Results", cols, table_flags)) {
				// Unpack the raw data we previously generated
				ad_unpack_context unpack;
				unpack_ctx_init(&unpack, anomaly.input_data, anomaly.input_data_size);
				
				ad_feature* feature = nullptr;
				void* data = nullptr;

				// Iterate until there is no more raw data to unpack
				while (!unpack_ctx_done(&unpack)) {
					unpack_ctx_next(&unpack, &feature, &data);
					if (row >= 0) {
						ImU32 row_bg_color = ImGui::GetColorU32(cluster_colors[this_row_cluster]);
						ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1, row_bg_color);
					}
							
					if (feature->type == ad_feature_type::ad_row) {
						if (row >= 0) {
							ImGui::TableNextColumn();
							ImGui::Text("%d", this_row_cluster);
						}
						row++;
						this_row_cluster = (int32)anomaly.results[row];
						ImGui::TableNextRow();
					}
					else if (feature->type == ad_feature_type::ad_float) {
						ImGui::TableNextColumn();
						ImGui::Text("%f", *(float32*)data);
					}
					else if (feature->type == ad_feature_type::ad_path) {
						ImGui::TableNextColumn();
						ImGui::Text((char*)data);
					}
				}

				// Get the last row
				ImGui::TableNextColumn();
				ImGui::Text("%d", this_row_cluster);

				ImGui::EndTable();
			}
			ImGui::End();
		}
		ImGui::End();

		global::file_browser.Display();

		ImGui::Render();


		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
  }

  // close GL context and any other GLFW resources
  glfwTerminate();
  return 0;
}
