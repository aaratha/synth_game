#include "render.h"
#include "oscillator.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

int windowWidth = 800;
int windowHeight = 600;

GLuint VBO, VAO;
GLuint shaderProgram;

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
uniform mat4 projection;
uniform mat4 model;
void main()
{
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 ourColor;
void main()
{
    FragColor = vec4(ourColor, 1.0);
}
)";

void compileShader(GLuint shader, const char* source)
{
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
}

void initRender()
{
    float vertices[] = {
        -0.5f,  0.5f,
         0.5f,  0.5f,
         0.5f, -0.5f,
        -0.5f, -0.5f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    compileShader(vertexShader, vertexShaderSource);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    compileShader(fragmentShader, fragmentShaderSource);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void render(const std::vector<Module>& Modules, int windowWidth, int windowHeight, int squareSize)
{
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f);
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    for (const Module& obj : Modules)
    {
        int x = obj.position_current.x;
        int y = obj.position_current.y;

        float centerX = x;
        float centerY = y;

        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(centerX, centerY, 0.0f));
        model = glm::scale(model, glm::vec3(squareSize, squareSize, 1.0f));
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        glUniform3f(glGetUniformLocation(shaderProgram, "ourColor"), 0.0f, 1.0f, 0.0f);
        glDrawArrays(GL_POLYGON, 0, 4);

        glUniform3f(glGetUniformLocation(shaderProgram, "ourColor"), 1.0f, 0.0f, 0.0f);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

