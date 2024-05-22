#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <eigen3/Eigen/Dense>
#include <SFML/Graphics.hpp>
#include <cmath>

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);


int windowWidth = 800;
int windowHeight = 600;

bool isMouseHeld = false;
bool isSpacePressed = false;

double dragForce = 4000;

double damping = 200;


using Vec2 = sf::Vector2f;

Vec2 normalize(const Vec2& vector)
{
    float length = std::sqrt(vector.x * vector.x + vector.y * vector.y);
    if (length != 0)
    {
        return Vec2(vector.x / length, vector.y / length);
    }
    return vector;
}


struct physicsObject
{
    Vec2 velocity;
    Vec2 position_current;
    Vec2 position_old;
    Vec2 acceleration;

    void updatePosition(float dt)
    {
        velocity = position_current - position_old;
        position_old = position_current;
        position_current = position_current + velocity + acceleration * dt * dt;
        acceleration = {};

        // std::cout << position_current.x << position_current.y;
    }

    void accelerate(Vec2 acc)
    {
        acceleration += acc;
    }
};


double xpos, ypos;

std::vector<physicsObject> physicsObjects;


void instantiate(double x, double y)
{
    physicsObject obj;
    obj.position_current = Vec2(x,y);
    obj.position_old = Vec2(x,y);
    obj.acceleration = Vec2(0,0);

    physicsObjects.push_back(obj);
}


void updatePositions(std::vector<physicsObject>& physicsObjects, float dt)
{
    for (auto& obj : physicsObjects)
    {
        obj.updatePosition(dt);
    }
}


void applyDrag(std::vector<physicsObject>& physicsObjects)
{
    for (auto& obj: physicsObjects)
    {
        Vec2 acceleration;
        if (isMouseHeld)
        {
            Vec2 dirToMouse = normalize(Vec2(xpos, ypos) - obj.position_current);
            acceleration = Vec2(dragForce * dirToMouse.x, dragForce * dirToMouse.y);
        }
        obj.accelerate(acceleration);
    }
}


void applyDamping(std::vector<physicsObject>& physicsObjects)
{
    for (auto& obj: physicsObjects)
    {
        obj.accelerate(-Vec2(obj.velocity.x * damping, obj.velocity.y * damping));
    }
}


void physicsProcess(std::vector<physicsObject>& physicsObjects, float dt)
{

    const int sub_steps = 2;
    const float sub_dt = dt / sub_steps;

    // the secret sauce: substeps...
    for (int i = 0; i < sub_steps; i++)
    {
        updatePositions(physicsObjects, dt);
        applyDrag(physicsObjects);
        applyDamping(physicsObjects);
        // applyGravity();
        // applyConstraint();
        // solveCollisions();
    }
}


void render()
{
    for (physicsObject& obj : physicsObjects)
    {
        // Pixel coordinates for the square
        int squareSize = 100;
        int x = obj.position_current.x; // X position in pixels
        int y = obj.position_current.y; // Y position in pixels

        // Calculate the center of the square
        float centerX = x - squareSize / 2.0f;
        float centerY = y - squareSize / 2.0f;

        // Convert pixel coordinates to normalized device coordinates
        float left = 2.0f * centerX / windowWidth - 1.0f;
        float right = 2.0f * (centerX + squareSize) / windowWidth - 1.0f;
        float top = 1.0f - 2.0f * centerY / windowHeight;
        float bottom = 1.0f - 2.0f * (centerY + squareSize) / windowHeight;

        glBegin(GL_POLYGON);
            glVertex2f(left, top);
            glVertex2f(right, top);
            glVertex2f(right, bottom);
            glVertex2f(left, bottom);
        glEnd();
    }
}



static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    ::xpos = xpos;
    ::ypos = ypos;
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        isMouseHeld = true;
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        isMouseHeld = false;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        isSpacePressed = true;
        instantiate(xpos, ypos);
    }
    else
        isSpacePressed = false;
}


int main()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // For MacOS
    #endif

    // Create a windowed mode window and its OpenGL context
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "GLFW OpenGL Game", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glViewport(0, 0, windowWidth, windowHeight);

    // Callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);




    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        float dt = 0.016f;
        physicsProcess(physicsObjects,dt);
        render();

        // Debug checks
        // std::cout << "Cursor Position: (" << xpos << ", " << ypos << ")" << std::endl;
        // std::cout << "isMouseHeld: " << isMouseHeld << std::endl;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}


// Function to process input
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}


// Callback function to adjust the viewport size when the window size changes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}


