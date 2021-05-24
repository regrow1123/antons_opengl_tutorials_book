#pragma once

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <cstdarg>

#define GL_LOG_FILE "gl.log"

extern int g_gl_width;
extern int g_gl_height;

extern GLFWwindow* g_window;

bool start_gl();

bool restart_gl_log();

bool gl_log(const char* message, ...);

bool gl_log_err(const char* message, ...);

void glfw_error_callback(int error, const char* description);

void log_gl_params();

void _update_fps_counter(GLFWwindow* window);

const char* GL_type_to_string(unsigned int type);

void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height);

void print_shader_info_log(GLuint shader_index);

void print_programme_info_log(GLuint sp);

void print_all(GLuint sp);

bool is_valid(GLuint sp);

bool parse_file_into_str(const char* file_name, char* shader_str, int max_len);
