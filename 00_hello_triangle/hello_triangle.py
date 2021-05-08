import glfw
from OpenGL.GL import *
import numpy as np

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

    if not glfw.init():
        print('ERROR: could not start GLFW3')
        return
    
    glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, 4)
    glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, 1)
    glfw.window_hint(glfw.OPENGL_FORWARD_COMPAT, GL_TRUE)
    glfw.window_hint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)

    window = glfw.create_window(640, 480, 'Hello Triangle', None, None)
    if not window:
        print('ERROR: could not open window with GLFW3')
        glfw.terminate()
        return
    glfw.make_context_current(window)
    renderer = glGetString(GL_RENDERER)
    version = glGetString(GL_VERSION)
    print('Renderer: ', renderer)
    print('OpenGL version supported ', version)

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

    while not glfw.window_should_close(window):
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT)
        glUseProgram(shader_programme)
        glBindVertexArray(vao)
        glDrawArrays(GL_TRIANGLES, 0, 3)
        glfw.poll_events()
        glfw.swap_buffers(window)

    glfw.terminate()
    return

if __name__=='__main__':
    main()
