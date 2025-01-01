#include "lab_m1/Tema2/main/Tema2.h"

#include <iostream>

using namespace std;
using namespace m1;

#define SKY_COLOR         0.6705, 0.788, 0.8529

Tema2::Tema2()
{
}

Tema2::~Tema2()
{
}

void Tema2::Init()
{
    // initialize propellers angle
    drone.propellersAngle = 0;
    drone.AngleOY = 0;

    // initialize drone coordinates
    drone.position = glm::vec3(0, 0, 0);

	AddDroneMesh();
    AddDronePropellerMesh();
}


void Tema2::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(SKY_COLOR, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();

    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}

void Tema2::FrameEnd()
{
    DrawCoordinateSystem();
}

void Tema2::Update(float deltaTimeSeconds)
{
    //glLineWidth(3);
    //glPointSize(5);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glm::mat4 droneMatrix = glm::mat4(1);
    droneMatrix *= transform3D::Translate(drone.position.x, drone.position.y, drone.position.z);
    droneMatrix *= transform3D::RotateOY(drone.AngleOY);

    // render the drone
    RenderMesh(meshes["drone"], shaders["VertexColor"], droneMatrix);

    // render propeller 1
    glm::mat4 propeller1Matrix = glm::mat4(1);
    propeller1Matrix *= transform3D::Translate(drone.position.x, drone.position.y, drone.position.z);
    propeller1Matrix *= transform3D::RotateOY(drone.AngleOY);
    propeller1Matrix *= transform3D::Translate(-0.9f, 0.34f, 0);
    propeller1Matrix *= transform3D::RotateOY(drone.propellersAngle);

    RenderMesh(meshes["drone-propeller"], shaders["VertexColor"], propeller1Matrix);

    // render propeller 2
    glm::mat4 propeller2Matrix = glm::mat4(1);
    propeller2Matrix *= transform3D::Translate(drone.position.x, drone.position.y, drone.position.z);
    propeller2Matrix *= transform3D::RotateOY(drone.AngleOY);
    propeller2Matrix *= transform3D::Translate(0.9f, 0.34f, 0);
    propeller2Matrix *= transform3D::RotateOY(drone.propellersAngle);

    RenderMesh(meshes["drone-propeller"], shaders["VertexColor"], propeller2Matrix);

    // render propeller 3
    glm::mat4 propeller3Matrix = glm::mat4(1);
    propeller3Matrix *= transform3D::Translate(drone.position.x, drone.position.y, drone.position.z);
    propeller3Matrix *= transform3D::RotateOY(drone.AngleOY);
    propeller3Matrix *= transform3D::Translate(0, 0.34f, -0.9f);
    propeller3Matrix *= transform3D::RotateOY(drone.propellersAngle);

    RenderMesh(meshes["drone-propeller"], shaders["VertexColor"], propeller3Matrix);

    // render propeller 4
    glm::mat4 propeller4Matrix = glm::mat4(1);
    propeller4Matrix *= transform3D::Translate(drone.position.x, drone.position.y, drone.position.z);
    propeller4Matrix *= transform3D::RotateOY(drone.AngleOY);
    propeller4Matrix *= transform3D::Translate(0, 0.34f, 0.9f);
    propeller4Matrix *= transform3D::RotateOY(drone.propellersAngle);

    RenderMesh(meshes["drone-propeller"], shaders["VertexColor"],propeller4Matrix);

    // update propellers angle
    if (drone.propellersAngle >= 2 * 3.14) {
        drone.propellersAngle = 0;
    } else {
        drone.propellersAngle += 10 * deltaTimeSeconds;
    }
}

void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    // go left
    if (window->KeyHold(GLFW_KEY_A) == true) {
        drone.position.x -= 2*deltaTime;
    }

    // go right
    if (window->KeyHold(GLFW_KEY_D) == true) {
        drone.position.x += 2*deltaTime;
    }

    // go front
    if (window->KeyHold(GLFW_KEY_W) == true) {
        drone.position.z -= 2*deltaTime;
    }

    // go back
    if (window->KeyHold(GLFW_KEY_S) == true) {
        drone.position.z += 2*deltaTime;
    }

    // go up
    if (window->KeyHold(GLFW_KEY_Q) == true) {
        drone.position.y += 2*deltaTime;
    }

    // go down
    if (window->KeyHold(GLFW_KEY_E) == true) {
        drone.position.y -= 2*deltaTime;
    }

    // rotate right
    if (window->KeyHold(GLFW_KEY_R) == true) {
        drone.AngleOY -= 2 * deltaTime;
    }

    // rotate left
    if (window->KeyHold(GLFW_KEY_T) == true) {
        drone.AngleOY += 2 * deltaTime;
    }
}

void Tema2::OnKeyPress(int key, int mods)
{
}

void Tema2::OnKeyRelease(int key, int mods)
{
}


void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
}


void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
}


void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
}

void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void Tema2::OnWindowResize(int width, int height)
{
}

