#include "gl_utils.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <ctime>

#define GL_LOG_FILE "gl.log"
#define MAX_SHADER_LENGTH 262144

bool restart_gl_log() {
	FILE* file = fopen(GL_LOG_FILE, "w");
	if (!file) {
		fprintf(stderr, "ERROR: could not open GL_LOG_FILE log file %s for writing\n", GL_LOG_FILE);
		return false;
	}

	time_t now = time(NULL);
	char* date = ctime(&now);
	fprintf(file, "GL_LOG_FILE log. local time %s\n", date);
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

bool start_gl() {
	gl_log("starting GLFW %s", glfwGetVersionString());

	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	g_window = glfwCreateWindow(g_gl_width, g_gl_height, "Extended Init.", NULL, NULL);
	if (!g_window) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetFramebufferSizeCallback(g_window, glfw_framebuffer_size_callback);
	glfwMakeContextCurrent(g_window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		fprintf(stderr, "Failed to initialize GLAD\n");
		glfwTerminate();
		return false;
	}

	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);
	gl_log("renderer: %s\nversion: %s\n", renderer, version);

	return true;
}

void glfw_error_callback(int error, const char* description) {
	fputs(description, stderr);
	gl_log_err("%s\n", description);
}

void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	g_gl_width = width;
	g_gl_height = height;
	printf("width %i height %i\n", width, height);
}

void _update_fps_counter(GLFWwindow* window) {
	static double previous_seconds = glfwGetTime();
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

bool parse_file_into_str(const char* file_name, char* shader_str, int max_len) {
	FILE* file = fopen(file_name, "r");
	if (!file) {
		gl_log_err("ERROR: opening file for reading: %s\n", file_name);
		return false;
	}

	size_t cnt = fread(shader_str, 1, max_len - 1, file);
	if ((int)cnt >= max_len - 1) {
		gl_log_err("WARNING: file %s too big - truncated.\n", file_name);
	}
	if (ferror(file)) {
		gl_log_err("ERROR: reading shader file %s\n", file_name);
		fclose(file);
		return false;
	}

	shader_str[cnt] = 0;
	fclose(file);
	return true;
}

void print_shader_info_log(GLuint shader_index) {
	int max_length = 2048;
	int actual_length = 0;
	char log[2048];
	glGetShaderInfoLog(shader_index, max_length, &actual_length, log);
	printf("shader info log for GL index %i: \n%s\n", shader_index, log);
	gl_log("shader info log for GL index %i: \n%s\n", shader_index, log);
}

bool create_shader(const char* file_name, GLuint* shader, GLenum type) {
	gl_log("creating shader form %s...\n", file_name);
	char shader_string[MAX_SHADER_LENGTH];
	parse_file_into_str(file_name, shader_string, MAX_SHADER_LENGTH);
	*shader = glCreateShader(type);
	const GLchar* p = (const GLchar*)shader_string;
	glShaderSource(*shader, 1, &p, NULL);
	glCompileShader(*shader);

	int params = -1;
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &params);
	if (params != GL_TRUE) {
		gl_log_err("ERROR: GL shader index %i did not compile\n", *shader);
		print_shader_info_log(*shader);
		return false;
	}
	gl_log("shader compile. index %i\n", *shader);
	return true;
}

void print_programme_info_log(GLuint sp) {
	int max_length = 2048;
	int actual_length = 0;
	char log[2048];
	glGetProgramInfoLog(sp, max_length, &actual_length, log);
	printf("program info log for GL index %u:\n%s", sp, log);
	gl_log("program info log for GL index %u:\n%s", sp, log);
}

bool is_programme_valid(GLuint sp) {
	glValidateProgram(sp);
	GLint params = -1;
	glGetProgramiv(sp, GL_VALIDATE_STATUS, &params);
	if (params != GL_TRUE) {
		gl_log_err("program %i GL_VALIDATE_STATUS = GL_FALSE\n", sp);
		print_programme_info_log(sp);
		return false;
	}
	gl_log("program %i GL_VALIDATE_STATUS = GL_TRUE\n", sp);
	return true;
}

bool create_programme(GLuint vert, GLuint frag, GLuint* programme) {
	*programme = glCreateProgram();
	gl_log("created programme %u. attaching shaders %u and %u...\n", *programme, vert, frag);
	glAttachShader(*programme, vert);
	glAttachShader(*programme, frag);
	glLinkProgram(*programme);
	GLint params = -1;
	glGetProgramiv(*programme, GL_LINK_STATUS, &params);
	if (params != GL_TRUE) {
		gl_log_err("ERROR: could not link shader programme GL index%u\n", *programme);
		print_programme_info_log(*programme);
		return false;
	}
	is_programme_valid(*programme);
	glDeleteShader(vert);
	glDeleteShader(frag);
	return true;
}

GLuint create_programme_from_files(const char* vert_file_name, const char* frag_file_name) {
	GLuint vert, frag, programme;
	create_shader(vert_file_name, &vert, GL_VERTEX_SHADER);
	create_shader(frag_file_name, &frag, GL_FRAGMENT_SHADER);
	create_programme(vert, frag, &programme);
	return programme;
}
