#include "gl_utils.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#define GL_LOG_FILE "gl.log"

int g_gl_width = 640;
int g_gl_height = 480;
GLFWwindow* g_window = NULL;

int main() {
	restart_gl_log();
	start_gl();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	GLfloat points[] = { 0.0f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f };
	GLfloat colours[] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };

	GLuint points_vbo;
	glGenBuffers(1, &points_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), points, GL_STATIC_DRAW);

	GLuint colours_vbo;
	glGenBuffers(1, &colours_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, colours_vbo);
	glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(GLfloat), colours, GL_STATIC_DRAW);

	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindBuffer(GL_ARRAY_BUFFER, colours_vbo);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	char vertex_shader[1024 * 256];
	char fragment_shader[1024 * 256];
	parse_file_into_str("test_vs.glsl", vertex_shader, 1024 * 256);
	parse_file_into_str("test_fs.glsl", fragment_shader, 1024 * 256);

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	const GLchar* p = (const GLchar*)vertex_shader;
	glShaderSource(vs, 1, &p, NULL);
	glCompileShader(vs);

	int params = -1;
	glGetShaderiv(vs, GL_COMPILE_STATUS, &params);
	if (params != GL_TRUE) {
		fprintf(stderr, "ERROR: GL shader index %i did not compile\n", vs);
		print_shader_info_log(vs);
		return 1;
	}

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	p = (const GLchar*)fragment_shader;
	glShaderSource(fs, 1, &p, NULL);
	glCompileShader(fs);

	glGetShaderiv(fs, GL_COMPILE_STATUS, &params);
	if (params != GL_TRUE) {
		fprintf(stderr, "ERROR: GL shader index %i did not compile\n", fs);
		print_shader_info_log(fs);
		return 1;
	}

	GLuint shader_programme = glCreateProgram();
	glAttachShader(shader_programme, vs);
	glAttachShader(shader_programme, fs);
	glLinkProgram(shader_programme);

	glGetProgramiv(shader_programme, GL_LINK_STATUS, &params);
	if (params != GL_TRUE) {
		fprintf(stderr, "ERROR: could not link shader programme GL index %i\n", shader_programme);
		print_programme_info_log(shader_programme);
		return false;
	}

	glEnable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	while (!glfwWindowShouldClose(g_window)) {
		_update_fps_counter(g_window);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, g_gl_width, g_gl_height);

		glUseProgram(shader_programme);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glfwPollEvents();
		if (glfwGetKey(g_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(g_window, 1);
		}
		glfwSwapBuffers(g_window);
	}
	glfwTerminate();
	return 0;
}