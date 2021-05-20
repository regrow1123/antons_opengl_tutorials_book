#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <ctime>

const char* GL_LOG_FILE = "gl.log";

bool restart_gl_log() {
	FILE* file = fopen(GL_LOG_FILE, "w");
	if (!file) {
		fprintf(stderr, "ERROR: could not open GL_LOG_FILE log file %s for writing\n", GL_LOG_FILE);
		return false;
	}

	time_t now = time(NULL);
	char* date = ctime(&now);
	fprintf(file, "GL_LOG_FILE log. local time %s", date);
	fprintf(file, "build version: %s %s\n\n", __DATE__, __TIME__);
	fclose(file);
	return true;
}

bool gl_log(const char* message, ...) {
	FILE* file = fopen(GL_LOG_FILE, "a");
	if (!file) {
		fprintf(stderr, "ERROR: could not open GL_LOG_FILE %s file for appending\n", GL_LOG_FILE);
		return false;
	}

	va_list argptr;
	va_start(argptr, message);
	vfprintf(file, message, argptr);
	va_end(argptr);
	fclose(file);
	return true;
}

bool gl_log_err(const char* message, ...) {
	FILE* file = fopen(GL_LOG_FILE, "a");
	if (!file) {
		fprintf(stderr, "ERROR: could not open GL_LOG_FILE %s file for appending\n", GL_LOG_FILE);
		return false;
	}

	va_list argptr;
	va_start(argptr, message);
	vfprintf(file, message, argptr);
	va_end(argptr);
	va_start(argptr, message);
	vfprintf(stderr, message, argptr);
	va_end(argptr);
	fclose(file);
	return true;
}

void glfw_error_callback(int error, const char* description) {
	gl_log_err("GLFW ERROR: code %i msg: %s\n", error, description);
}

//뷰포트나 커서 위치를 위해 윈도우 크기를 계속 추적해야함.
int g_gl_width = 640;
int g_gl_height = 480;

void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	g_gl_width = width;
	g_gl_height = height;
	//카메라 변경 등으로 인한 투영행렬 변경을 여기서 업데이트 가능.
}

void log_gl_params() {
	GLenum params[] = {
		GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
		GL_MAX_CUBE_MAP_TEXTURE_SIZE,
		GL_MAX_DRAW_BUFFERS,
		GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
		GL_MAX_TEXTURE_IMAGE_UNITS,
		GL_MAX_TEXTURE_SIZE,
		GL_MAX_VARYING_FLOATS,
		GL_MAX_VERTEX_ATTRIBS,
		GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
		GL_MAX_VERTEX_UNIFORM_COMPONENTS,
		GL_MAX_VIEWPORT_DIMS,
		GL_STEREO,
	};
	const char* names[] = {
		"GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
		"GL_MAX_CUBE_MAP_TEXTURE_SIZE",
		"GL_MAX_DRAW_BUFFERS",
		"GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
		"GL_MAX_TEXTURE_IMAGE_UNITS",
		"GL_MAX_TEXTURE_SIZE",
		"GL_MAX_VARYING_FLOATS",
		"GL_MAX_VERTEX_ATTRIBS",
		"GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
		"GL_MAX_VERTEX_UNIFORM_COMPONENTS",
		"GL_MAX_VIEWPORT_DIMS",
		"GL_STEREO",
	};
	gl_log("GL Context params:\n");
	for (int i = 0; i < 10; i++) {
		int v = 0;
		glGetIntegerv(params[i], &v);
		gl_log("%s %i\n", names[i], v);
	}

	int v[2] = { 0, 0 };
	glGetIntegerv(params[10], v);
	gl_log("%s %i %i\n", names[10], v[0], v[1]);
	unsigned char s = 0;
	glGetBooleanv(params[11], &s);
	gl_log("%s %i\n", names[11], (unsigned int)s);
	gl_log("-----------------------------\n");
}

static double previous_seconds;
void _update_fps_counter(GLFWwindow* window) {
	static int frame_count = 0;

	double current_seconds = glfwGetTime();
	double elapsed_seconds = current_seconds - previous_seconds;
	if (elapsed_seconds > 0.25) {
		previous_seconds = current_seconds;

		double fps = (double)frame_count / elapsed_seconds;
		char tmp[128];
		sprintf(tmp, "opengl @ fps: %.2f", fps);
		glfwSetWindowTitle(window, tmp);
		frame_count = 0;
	}
	frame_count++;
}

int main() {
	GLfloat points[] = { 0.0f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f };
	const char* vertex_shader =
		"#version 410\n"
		"in vec3 vp;"
		"void main() {"
		"  gl_Position = vec4(vp, 1.0);"
		"}";

	const char* fragment_shader =
		"#version 410\n"
		"out vec4 frag_colour;"
		"void main() {"
		"  frag_colour = vec4(0.5, 0.0, 0.5, 1.0);"
		"}";

	restart_gl_log();
	gl_log("starting GLFW\n%s\n", glfwGetVersionString());
	glfwSetErrorCallback(glfw_error_callback);
	
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return 1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);

	//GLFWmonitor* mon = glfwGetPrimaryMonitor();
	//const GLFWvidmode* vmode = glfwGetVideoMode(mon);
	//GLFWwindow* window = glfwCreateWindow(
	//	vmode->width, vmode->height, "Extended GL Init", mon, NULL
	//);

	GLFWwindow* window = glfwCreateWindow(g_gl_width, g_gl_height, "Extended Init.", NULL, NULL);
	if (!window) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		fprintf(stderr, "Failed to initialize GLAD\n");
		return 1;
	}

	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);
	gl_log("renderer: %s\nversion: %s\n", renderer, version);
	log_gl_params();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), points, GL_STATIC_DRAW);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &vertex_shader, NULL);
	glCompileShader(vs);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &fragment_shader, NULL);
	glCompileShader(fs);
	GLuint shader_programme = glCreateProgram();
	glAttachShader(shader_programme, vs);
	glAttachShader(shader_programme, fs);
	glLinkProgram(shader_programme);

	previous_seconds = glfwGetTime();
	while (!glfwWindowShouldClose(window)) {
		_update_fps_counter(window);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, g_gl_width, g_gl_height);

		glUseProgram(shader_programme);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwPollEvents();
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, 1);
		}
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}
