#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <iostream>

GLFWwindow* window;

// GLFW Callbacks
static void GLFW_Cursor_Pos_Callback(GLFWwindow* window, double xpos, double ypos) {
	std::cout << "cursor: " << std::to_string(xpos) << ", " << std::to_string(ypos) << std::endl;
}

void GLFW_Mouse_Button_Callback(GLFWwindow* window, int button, int action, int mods) {
	std::cout << "mouse: " << std::to_string(button) << ", " << std::to_string(action) << std::endl;
	}

void GLFW_Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	std::cout << "key: " << std::to_string(key) << ", " << std::to_string(action) << std::endl;
}

void GLFW_Scroll_Callback(GLFWwindow* window, double xoffset, double yoffset) {
	std::cout << "scroll: " << std::to_string(xoffset) << ", " << std::to_string(yoffset) << std::endl;
}

void GLFW_Error_Callback(int err, const char* msg) {
	std::cout << err;
	std::cout << msg;
}

void GLFW_Window_Size_Callback(GLFWwindow* window, int width, int height) {
	std::cout << "window size: " << std::to_string(width) << ", " << std::to_string(height) << std::endl;
}

int init_glfw() {
	auto result = glfwInit();
	
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac

	window = glfwCreateWindow(1280, 720, "Anomaly", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	
	glfwSetCursorPosCallback(window, GLFW_Cursor_Pos_Callback);
	glfwSetMouseButtonCallback(window, GLFW_Mouse_Button_Callback);
	glfwSetKeyCallback(window, GLFW_Key_Callback);
	glfwSetScrollCallback(window, GLFW_Scroll_Callback);
	glfwSetWindowSizeCallback(window, GLFW_Window_Size_Callback);
	
	glfwSwapInterval(0);

	return 0;
}
void init_gl() {
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
}
void init_imgui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    
	const char* glsl_version = "#version 150";
	ImGui_ImplOpenGL3_Init(glsl_version);
	
	auto& imgui = ImGui::GetIO();
	ImGui::StyleColorsDark();

	imgui.IniFilename = nullptr;
}

bool show_demo_window = true;
int main (int argc, char** argv) {
  init_glfw();
  init_gl();
  init_imgui();

  // get version info
  const GLubyte* renderer = glGetString (GL_RENDERER); // get renderer string
  const GLubyte* version = glGetString (GL_VERSION); // version as a string
  std::cout<<"Renderer: "<<renderer<<std::endl;
  std::cout<<"OpenGL version supported "<<version<<std::endl;

  while(!glfwWindowShouldClose(window)) {

		ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

		ImGui::ShowDemoWindow(&show_demo_window);

		ImGui::Begin("anomaly");
		ImGui::End();
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