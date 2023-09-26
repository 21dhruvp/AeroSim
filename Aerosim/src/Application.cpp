#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"

struct ShaderProgSource {
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgSource ParseShader(const std::string& filepath) {
    std::ifstream stm(filepath);

    enum class ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stm, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
        } else {
            ss[(int)type] << line << '\n';
        }
    }

    return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int len;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
        char* message = (char*)alloca(len * sizeof(char));
        glGetShaderInfoLog(id, len, &len, message);
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? " vertex" : " fragment") << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

static int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDetachShader(program, vs);
    glDetachShader(program, fs);
	glDeleteShader(vs);
    glDeleteShader(vs);

    return program;
}

int main(void)
{
    GLFWwindow* window;


    /* Initialize the library */
    if (!glfwInit()) {
        return -1;
    }

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Test", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    glClearColor(0.2f, 0.4f, 0.8f, 1.0f); // Set background to sky blue

    if (glewInit() != GLEW_OK)
        return -1;
    {
		GLfloat pos[] = {
			-0.5f, -0.5f,   //0
			0.5f, -0.5f,    //1
			0.5f, 0.5f,     //2
			-0.5f, 0.5f,    //3
		};

		GLuint indices[] = {
			0, 1, 2,
			2, 3, 0
		};

        VertexArray va;
		VertexBuffer vb(pos, 4 * 2 * sizeof(float));
        VertexBufferLayout layout;
        layout.Push<float>(2);
        va.AddBuffer(vb, layout);

		IndexBuffer ib(indices, 6);

		ShaderProgSource source = ParseShader("res/shaders/basic.shader");


		unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
		glUseProgram(shader);

		int location = glGetUniformLocation(shader, "u_Color");
		ASSERT(location != -1);
		glUniform4f(location, 0.2f, 0.3f, 0.8f, 1.0f);

        va.Unbind();
		glUseProgram(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		float r = 0.0f;
		float increment = 0.05f;
		/* Loop until the user closes the window */
		while (!glfwWindowShouldClose(window))
		{
			/* Render here */
			GLCall(glClear(GL_COLOR_BUFFER_BIT));

			GLCall(glUseProgram(shader));
			GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));

            va.Bind();
            ib.Bind();

			GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

			if (r > 1.0f)
				increment = -0.05f;
			else if (r < 0.0f)
				increment = 0.05f;

			r += increment;

			/* Swap front and back buffers */
			glfwSwapBuffers(window);

			/* Poll for and process events */
			glfwPollEvents();
    }

    glDeleteProgram(shader);
}
    glfwTerminate();
    return 0;
}   