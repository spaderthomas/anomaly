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
	bool show_imgui_demo = false;
	char install_dir [256] = {0};
};

int main (int argc, char** argv) {
	struct modals_t {
		bool save_config = false;
	};
	bool is_loading_config = false;

	init_paths();
  	init_glfw();
  	init_gl();
  	init_imgui();

	config_t config = {0};
  	while(!glfwWindowShouldClose(window)) {
		double frame_start_time = glfwGetTime();
		double dt = 1.f / 30.f;

		ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

		if (global::show_imgui_demo) ImGui::ShowDemoWindow(&global::show_imgui_demo);

		ImGui::SetNextWindowSize({400, 0});
		ImGui::Begin("anomaly");
		
		modals_t modals;	
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::BeginMenu("Configuration")) {
					if (ImGui::MenuItem("Load")) {
						global::file_browser.Open();
						is_loading_config = true;
					}
					if (ImGui::MenuItem("Save")) {
						modals.save_config = true;
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

		// Modal dialogue to open a popup to prompt the user for a name for this config
		if (modals.save_config) {
			ImGui::OpenPopup("Save Configuration");
		}
		if (ImGui::BeginPopupModal("Save Configuration")) {
			ImGui::Text("Name");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(200);
			ImGui::InputText("##main_menu:save_config:name", config.name, 256);

			if (ImGui::Button("Save")) {
				if (!strlen(config.name)) {
					std::cout << "Cannot save configuration without a name." << std::endl;
					ImGui::OpenPopup("Error");
				} else {
					char path [256];
					snprintf(path, 256, "%s/%s/%s.ini", "..", "data", config.name);
					cfg_save(&config, path);
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

		// If we asked to load a config, check if we selected a file and if so load it
		if (is_loading_config) {
			if (global::file_browser.HasSelected()) {
				auto path = global::file_browser.GetSelected().string();
				cfg_load(&config, path.c_str());
				global::file_browser.ClearSelected();
				is_loading_config = false;
			}

		}

		// Display the config fields for the generation and featurization
		ImGui::Text("ad_gen config");

		int input_size = 200;
		ImGui::Text("function");
		ImGui::SameLine();
		ImGui::SameLine(ImGui::GetWindowWidth() - input_size);
		ImGui::InputText("##ad_gen:function", config.generator_function, 64);
		
		ImGui::Text("raw data file");
		ImGui::SameLine();
		ImGui::SameLine(ImGui::GetWindowWidth() - input_size);
		ImGui::InputText("##ad_gen:raw_data_file", config.raw_data_file, 256);

		ImGui::Text("featurized data file");
		ImGui::SameLine();
		ImGui::SameLine(ImGui::GetWindowWidth() - input_size);
		ImGui::InputText("##ad_gen:featurized_data_file", config.featurized_data_file, 256);

		// Display the config fields for SOM itself
		ImGui::Separator();
		ImGui::Text("som config");

		ImGui::Text("neighborhood function");
		ImGui::SameLine(ImGui::GetWindowWidth() - input_size);
		ImGui::InputText("##som:neighborhood", config.neighborhood_function, 256);

		ImGui::Text("learning rate");
		ImGui::SameLine(ImGui::GetWindowWidth() - input_size);
		ImGui::InputFloat("##som:learning_rate", &config.learning_rate);

		ImGui::Text("# clusters");
		ImGui::SameLine(ImGui::GetWindowWidth() - input_size);
		ImGui::InputScalar("##som:clusters", ImGuiDataType_U32, &config.count_clusters);

		ImGui::Text("error threshold");
		ImGui::SameLine();
		ImGui::SameLine(ImGui::GetWindowWidth() - input_size);
		ImGui::InputFloat("##som:error", &config.error_threshold);

		ImGui::Text("seed");
		ImGui::SameLine();
		ImGui::SameLine(ImGui::GetWindowWidth() - input_size);
		ImGui::InputScalar("##som:seed", ImGuiDataType_U32, &config.seed);

		if (ImGui::Button("Train")) {
			// Generate
			uint32 buffer_size = 1024 * 1024;
			char* buffer = (char*)calloc(sizeof(char), buffer_size);
			ad_generate(&config, buffer, buffer_size);

			// Featurize
			ad_unpack_context unpack;
			unpack_ctx_init(&unpack, buffer, buffer_size);
			
			ad_featurized_header header;
			std::vector<float32> featurized;
			
			ad_featurize(&unpack, &featurized, &header);

			// Train
			som_t som;
			som.config = config;
			ad_train(som, &header, featurized.data());
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
