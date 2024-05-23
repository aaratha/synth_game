#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <eigen3/Eigen/Dense>
#include <SFML/Graphics.hpp>
#include <SDL2/SDL.h>
#include <cmath>

// build command:
//     c++ -o engine main.cpp -lGL -lGLEW -lglfw -lGLU -lsfml-graphics -lSDL2

// Function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

int windowWidth = 800;
int windowHeight = 600;

int squareSize = 100;
int deadZone = 30;

bool isMouseHeld = false;
bool isSpacePressed = false;

double dragForce = 4000;

double damping = 200;

// initial voltage
double V = 1;
const int SAMPLE_RATE = 44100;
double AMPLITUDE = 28000;
double FREQUENCY = 440;

using Vec2 = sf::Vector2f;


void audio_callback(void* userData, Uint8* stream, int len)
{
    static double phase = 0.0;
    double phaseIncrement = 2.0 * M_PI * FREQUENCY / SAMPLE_RATE;
    int16_t* buffer = reinterpret_cast<int16_t*>(stream);
    int samples = len / sizeof(int16_t);

    for (int i = 0; i < samples; i++)
    {
        buffer[i] = static_cast<int16_t>(AMPLITUDE * std::sin(phase));
        phase += phaseIncrement;
        if (phase >= 2.0 * M_PI)
            phase -= 2.0 * M_PI;
    }
}


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


int selectedObject = -1;

void applyDrag(std::vector<physicsObject>& physicsObjects)
{
    for (size_t i = 0; i < physicsObjects.size(); ++i)
    {
        auto& obj = physicsObjects[i];
        Vec2 distV = Vec2(xpos, ypos) - obj.position_current;
        double distance = std::sqrt(distV.x * distV.x + distV.y * distV.y);
        Vec2 direction = normalize(distV);

        if (isMouseHeld && (selectedObject == -1 || selectedObject == i))
        {
            if (distance < squareSize / 2.0f || selectedObject == i)
            {
                selectedObject = i;
                if (distance > deadZone)
                {
                    Vec2 acceleration = Vec2(dragForce * direction.x, dragForce * direction.y);
                    obj.accelerate(acceleration);
                }
                break;
            }
        }
    }
}


void applyDamping(std::vector<physicsObject>& physicsObjects)
{
    for (auto& obj: physicsObjects)
    {
        obj.accelerate(-Vec2(obj.velocity.x * damping, obj.velocity.y * damping));
    }
}


void solveCollisions(std::vector<physicsObject>& physicsObjects)
{
    const double restitution = 0.9; // Coefficient of restitution (bounciness)
    const double squareSize = 100.0; // Size of the square

    for (size_t i = 0; i < physicsObjects.size(); ++i)
    {
        for (size_t j = i + 1; j < physicsObjects.size(); ++j)
        {
            auto& obj1 = physicsObjects[i];
            auto& obj2 = physicsObjects[j];

            Vec2 pos1 = obj1.position_current;
            Vec2 pos2 = obj2.position_current;

            Vec2 delta = pos2 - pos1;
            double distSquared = delta.x * delta.x + delta.y * delta.y;
            double minDist = squareSize;
            double minDistSquared = minDist * minDist;

            if (distSquared < minDistSquared)
            {
                double dist = std::sqrt(distSquared);
                Vec2 collision_axis = (dist != 0.0) ? Vec2(delta.x / dist, delta.y / dist) : Vec2(1.0f, 0.0f); // Avoid division by zero

                // Adjust positions to resolve overlap
                double overlap = 0.5 * (minDist - dist);
                obj1.position_current -= Vec2(overlap * collision_axis.x, overlap * collision_axis.y);
                obj2.position_current += Vec2(overlap * collision_axis.x, overlap * collision_axis.y);

                // Calculate relative velocity
                Vec2 relativeVelocity = obj2.velocity - obj1.velocity;
                double velAlongNormal = relativeVelocity.x * collision_axis.x + relativeVelocity.y * collision_axis.y;

                // Do not resolve if velocities are separating
                if (velAlongNormal > 0)
                    continue;

                // Calculate impulse scalar
                double j = -(1 + restitution) * velAlongNormal;
                j /= 2; // Divide by the sum of the inverse masses (assuming equal mass)

                // Apply impulse
                Vec2 impulse = Vec2(j * collision_axis.x, j * collision_axis.x);
                obj1.velocity -= impulse;
                obj2.velocity += impulse;
            }
        }
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
        solveCollisions(physicsObjects);
        // applyGravity();
        // applyConstraint();
    }
}


void render()
{
    for (physicsObject& obj : physicsObjects)
    {
        // Pixel coordinates for the square
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
    {
        isMouseHeld = false;
        selectedObject = -1;
    }
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
    
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_AudioSpec desiredSpec;
    SDL_AudioSpec obtainedSpec;

    SDL_memset(&desiredSpec, 0, sizeof(desiredSpec));
    desiredSpec.freq = SAMPLE_RATE;
    desiredSpec.format = AUDIO_S16SYS;
    desiredSpec.channels = 1;
    desiredSpec.samples = 4096;
    desiredSpec.callback = audio_callback;

    if (SDL_OpenAudio(&desiredSpec, &obtainedSpec) < 0) {
        std::cerr << "Failed to open audio: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_PauseAudio(0); // Start playing audio

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

        // FREQUENCY = std::sqrt(xpos*xpos + ypos*ypos);
    }
    SDL_CloseAudio();
    SDL_Quit();
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
