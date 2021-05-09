import glfw
from OpenGL.GL import *
import numpy as np
import time
import os
import sys

GL_LOG_FILE = 'gl.log'

def restart_gl_log():
    file = open(GL_LOG_FILE, 'w')
    if not file:
        print(f'ERROR: could not open GL_LOG_FILE log file {GL_LOG_FILE} for writing', file=sys.stderr)
        return False
    
    file.write(f'GL_LOG_FILE log. local time {time.strftime("%c", time.localtime(time.time()))}\n')
    file.write(f'{time.ctime(os.path.getmtime(__file__))}\n')
    file.close()
    return True

def gl_log(message):
    file = open(GL_LOG_FILE, 'a')
    if not file:
        print(f'ERROR: could not open GL_LOG_FILE {GL_LOG_FILE} file for appending', file=sys.stderr)
        return False

    file.write(message)
    return True

def gl_log_err(message):
    file = open(GL_LOG_FILE, 'a')
    if not file:
        print(f'ERROR: could not open GL_LOG_FILE {GL_LOG_FILE} file for appending', file=sys.stderr)
        return False
    
    file.write(message)
    print(message, file=sys.stderr)

def glfw_error_callback(error, description):
    gl_log_err(f'GLFW ERROR: code {error} msg: {description}')

g_gl_width, g_gl_height = 640, 480

def glfw_framebuffer_size_callback(window, width, height):
    g_gl_width, g_gl_height = width, height
    print(f'width {width} height {height}')

def log_gl_params():
    params = [
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
    ]
    names = [
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
    ]
    gl_log('GL Context Params:\n')
    for i in range(10):
        v = glGetIntegerv(params[i])
        gl_log(f'{names[i]} {v}\n')
    
    v = glGetIntegerv(params[10])
    gl_log(f'{names[10]} {v[0]} {v[1]}\n')

    s = glGetBooleanv(params[11])
    gl_log(f'{names[11]} {s}\n')
    gl_log('-----------------------------\n')

def _update_fps_counter(window, previous_seconds, frame_count):
    current_seconds = glfw.get_time()
    elapsed_seconds = current_seconds - previous_seconds
    if elapsed_seconds > 0.25:
        previous_seconds = current_seconds

        fps = frame_count/elapsed_seconds
        glfw.set_window_title(window, f'opengl @ fps {fps:.2f}')
        frame_count = 0
    frame_count += 1
    return previous_seconds, frame_count

def main():
    points = [0.0, 0.5, 0.0, 0.5, -0.5, 0.0, -0.5, -0.5, 0.0]
    points = np.array(points, dtype=np.float32)

    vertex_shader = '''
        #version 410
        in vec3 vp;
        void main() {
            gl_Position = vec4(vp, 1.0);
        }
        '''

    fragment_shader = '''
        #version 410
        out vec4 frag_colour;
        void main() {
            frag_colour = vec4(0.5, 0.0, 0.5, 1.0);
        }
        '''

    restart_gl_log()
    gl_log(f'starting GLFW\n{glfw.get_version_string()}\n')
    glfw.set_error_callback(glfw_error_callback)
    if not glfw.init():
        print('ERROR: could not start GLFW3', file=sys.stderr)
        return 1
    
    glfw.window_hint(glfw.SAMPLES, 4)
    '''
    mon = glfw.get_primary_monitor()
    vmode = glfw.get_video_mode(mon)
    window = glfw.create_window(vmode.width, vmode.height, 'Extended GL Init', mon, None)
    '''
    window = glfw.create_window(g_gl_width, g_gl_height, 'Extended GL Init', None, None)
    if not window:
        print('ERROR: could not open window with GLFW3', file=sys.stderr)
        glfw.terminate()
        return
    glfw.set_framebuffer_size_callback(window, glfw_framebuffer_size_callback)
    glfw.make_context_current(window)

    renderer = glGetString(GL_RENDERER)
    version = glGetString(GL_VERSION)
    print('Renderer: ', renderer)
    print('OpenGL version supported ', version)
    gl_log(f'renderer: {renderer}\nversion: {version}\n')
    log_gl_params()

    glEnable(GL_DEPTH_TEST)
    glDepthFunc(GL_LESS)

    vbo = glGenBuffers(1)
    glBindBuffer(GL_ARRAY_BUFFER, vbo)
    glBufferData(GL_ARRAY_BUFFER, 9*sizeof(GLfloat), points, GL_STATIC_DRAW)

    vao = glGenVertexArrays(1)
    glBindVertexArray(vao)
    glEnableVertexAttribArray(0)
    glBindBuffer(GL_ARRAY_BUFFER, vbo)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, None)

    vert_shader = glCreateShader(GL_VERTEX_SHADER)
    glShaderSource(vert_shader, vertex_shader)
    glCompileShader(vert_shader)
    frag_shader = glCreateShader(GL_FRAGMENT_SHADER)
    glShaderSource(frag_shader, fragment_shader)
    glCompileShader(frag_shader)
    shader_programme = glCreateProgram()
    glAttachShader(shader_programme, frag_shader)
    glAttachShader(shader_programme, vert_shader)
    glLinkProgram(shader_programme)


    previous_seconds, frame_count = glfw.get_time(), 0
    while not glfw.window_should_close(window):
        previous_seconds, frame_count = _update_fps_counter(window, previous_seconds, frame_count)
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT)
        glViewport(0, 0, g_gl_width, g_gl_height)

        glUseProgram(shader_programme)
        glBindVertexArray(vao)
        glDrawArrays(GL_TRIANGLES, 0, 3)
        glfw.poll_events()
        if glfw.PRESS == glfw.get_key(window, glfw.KEY_ESCAPE):
            glfw.set_window_should_close(window, 1)

        glfw.swap_buffers(window)

    glfw.terminate()
    return

if __name__=='__main__':
    main()