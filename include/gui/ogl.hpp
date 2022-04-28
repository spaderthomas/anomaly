#ifndef AD_OGL_H
#define AD_OGL_H
extern GLFWwindow* window;

void GLFW_Cursor_Pos_Callback(GLFWwindow* window, double xpos, double ypos);
void GLFW_Mouse_Button_Callback(GLFWwindow* window, int button, int action, int mods);
void GLFW_Key_Callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void GLFW_Scroll_Callback(GLFWwindow* window, double xoffset, double yoffset);
void GLFW_Error_Callback(int err, const char* msg);
void GLFW_Window_Size_Callback(GLFWwindow* window, int width, int height);

int init_glfw();
void init_gl();
void init_imgui();
#endif