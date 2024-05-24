#include "physics.h"
#include "constants.h"
#include "render.h"
#include <cmath>
#include <vector>

double xpos, ypos;
int deadZone = 30;
bool isMouseHeld = false;
bool isSpacePressed = false;
double dragForce = 4000;
double damping = 400;
std::vector<Module> Modules;
int selectedObject = -1;

Vec2 normalize(const Vec2& vector)
{
    float length = std::sqrt(vector.x * vector.x + vector.y * vector.y);
    if (length != 0)
    {
        return Vec2(vector.x / length, vector.y / length);
    }
    return vector;
}

Module::Module(int freq, int amp) : frequency(freq), amplitude(amp), phase(0.0) {}

void Module::generateSound(double dt)
{
    double phaseIncrement = 2.0 * M_PI * frequency / SAMPLE_RATE;
    phase += phaseIncrement * dt;
    if (phase >= 2.0 * M_PI)
        phase -= 2.0 * M_PI;
}

void physicsObject::updatePosition(float dt)
{
    velocity = position_current - position_old;
    position_old = position_current;
    position_current = position_current + velocity + acceleration * dt * dt;
    acceleration = {};
}

void physicsObject::accelerate(Vec2 acc)
{
    acceleration += acc;
}

void instantiate(double x, double y)
{
    Module obj(440, 28000);
    obj.position_current = Vec2(x, y);
    obj.position_old = Vec2(x, y);
    obj.acceleration = Vec2(0, 0);

    Modules.push_back(obj);
}

void updatePositions(std::vector<Module>& Modules, float dt)
{
    for (auto& obj : Modules)
    {
        obj.updatePosition(dt);
    }
}

void applyDrag(std::vector<Module>& Modules)
{
    for (size_t i = 0; i < Modules.size(); ++i)
    {
        auto& obj = Modules[i];
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

void applyDamping(std::vector<Module>& Modules)
{
    for (auto& obj : Modules)
    {
        obj.accelerate(-Vec2(obj.velocity.x * damping, obj.velocity.y * damping));
    }
}

void solveCollisions(std::vector<Module>& Modules)
{
    const double restitution = 0.9;
    const double squareSize = 100.0;

    for (size_t i = 0; i < Modules.size(); ++i)
    {
        for (size_t j = i + 1; j < Modules.size(); ++j)
        {
            auto& obj1 = Modules[i];
            auto& obj2 = Modules[j];

            Vec2 pos1 = obj1.position_current;
            Vec2 pos2 = obj2.position_current;

            Vec2 delta = pos2 - pos1;
            double distSquared = delta.x * delta.x + delta.y * delta.y;
            double minDist = squareSize;
            double minDistSquared = minDist * minDist;

            if (distSquared < minDistSquared)
            {
                double dist = std::sqrt(distSquared);
                Vec2 collision_axis = (dist != 0.0) ? Vec2(delta.x / dist, delta.y / dist) : Vec2(1.0f, 0.0f);

                double overlap = 0.5 * (minDist - dist);
                obj1.position_current -= Vec2(overlap * collision_axis.x, overlap * collision_axis.y);
                obj2.position_current += Vec2(overlap * collision_axis.x, overlap * collision_axis.y);

                Vec2 relativeVelocity = obj2.velocity - obj1.velocity;
                double velAlongNormal = relativeVelocity.x * collision_axis.x + relativeVelocity.y * collision_axis.y;

                if (velAlongNormal > 0)
                    continue;

                double j = -(1 + restitution) * velAlongNormal;
                j /= 2;

                Vec2 impulse = Vec2(j * collision_axis.x, j * collision_axis.y);
                obj1.velocity -= impulse;
                obj2.velocity += impulse;
            }
        }
    }
}

void physicsProcess(std::vector<Module>& Modules, float dt)
{
    const int sub_steps = 2;
    const float sub_dt = dt / sub_steps;

    for (int i = 0; i < sub_steps; i++)
    {
        updatePositions(Modules, sub_dt);
        applyDrag(Modules);
        applyDamping(Modules);
        solveCollisions(Modules);
    }
}

