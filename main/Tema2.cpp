#include "lab_m1/Tema2/main/Tema2.h"

#include <iostream>

using namespace std;
using namespace m1;

#define SKY_COLOR         0.6705, 0.788, 0.8529

#define CAMERA_TO_DRONE_DIST_OX 6.5
#define CAMERA_TO_DRONE_DIST_OY 1.25

Tema2::Tema2()
{
}

Tema2::~Tema2()
{
}

void Tema2::Init()
{
    // initialize the drone camera
    //camera = new implemented::DroneCamera();
    //camera->Set(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), glm::vec3(0, 1, 0));

    //float fov = RADIANS(100);
    //float zNear = 0.01f;
    //float zFar = 100.0f;

    //projectionMatrix = glm::perspective(fov, window->props.aspectRatio, zNear, zFar);

    // initialize propellers angle
    drone.propellersAngle = 0;
    drone.angleOY = 0;

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
    droneMatrix *= transform3D::RotateOY(drone.angleOY);
    droneMatrix *= transform3D::RotateOY(0.785f);

    // render the drone
    RenderMesh(meshes["drone"], shaders["VertexColor"], droneMatrix);

    // render propeller 1
    glm::mat4 propeller1Matrix = glm::mat4(1);
    propeller1Matrix *= transform3D::Translate(drone.position.x, drone.position.y, drone.position.z);
    propeller1Matrix *= transform3D::RotateOY(drone.angleOY);
    propeller1Matrix *= transform3D::RotateOY(0.785f);
    propeller1Matrix *= transform3D::Translate(-0.9f, 0.34f, 0);
    propeller1Matrix *= transform3D::RotateOY(drone.propellersAngle);

    RenderMesh(meshes["drone-propeller"], shaders["VertexColor"], propeller1Matrix);

    // render propeller 2
    glm::mat4 propeller2Matrix = glm::mat4(1);
    propeller2Matrix *= transform3D::Translate(drone.position.x, drone.position.y, drone.position.z);
    propeller2Matrix *= transform3D::RotateOY(drone.angleOY);
    propeller2Matrix *= transform3D::RotateOY(0.785f);
    propeller2Matrix *= transform3D::Translate(0.9f, 0.34f, 0);
    propeller2Matrix *= transform3D::RotateOY(drone.propellersAngle);

    RenderMesh(meshes["drone-propeller"], shaders["VertexColor"], propeller2Matrix);

    // render propeller 3
    glm::mat4 propeller3Matrix = glm::mat4(1);
    propeller3Matrix *= transform3D::Translate(drone.position.x, drone.position.y, drone.position.z);
    propeller3Matrix *= transform3D::RotateOY(drone.angleOY);
    propeller3Matrix *= transform3D::RotateOY(0.785f);
    propeller3Matrix *= transform3D::Translate(0, 0.34f, -0.9f);
    propeller3Matrix *= transform3D::RotateOY(drone.propellersAngle);

    RenderMesh(meshes["drone-propeller"], shaders["VertexColor"], propeller3Matrix);

    // render propeller 4
    glm::mat4 propeller4Matrix = glm::mat4(1);
    propeller4Matrix *= transform3D::Translate(drone.position.x, drone.position.y, drone.position.z);
    propeller4Matrix *= transform3D::RotateOY(drone.angleOY);
    propeller4Matrix *= transform3D::RotateOY(0.785f);
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
    // compute drone's forward and right directions
    glm::vec3 forward = glm::vec3(sin(drone.angleOY), 0, cos(drone.angleOY));
    glm::vec3 right = glm::vec3(cos(drone.angleOY), 0, -sin(drone.angleOY));

    // go left
    if (window->KeyHold(GLFW_KEY_A) == true) {
        drone.position -= right * 4.0f * deltaTime;
    }

    // go right
    if (window->KeyHold(GLFW_KEY_D) == true) {
        drone.position += right * 4.0f * deltaTime;
    }

    // go front
    if (window->KeyHold(GLFW_KEY_W) == true) {
        drone.position -= forward * 4.0f * deltaTime;
    }

    // go back
    if (window->KeyHold(GLFW_KEY_S) == true) {
        drone.position += forward * 4.0f * deltaTime;
    }

    // go up
    if (window->KeyHold(GLFW_KEY_Q) == true) {
        drone.position.y -= 4 * deltaTime;
    }

    // go down
    if (window->KeyHold(GLFW_KEY_E) == true) {
        drone.position.y += 4 * deltaTime;
    }

    // rotate right
    if (window->KeyHold(GLFW_KEY_R) == true) {
        drone.angleOY += 2 * deltaTime;
    }

    // rotate left
    if (window->KeyHold(GLFW_KEY_T) == true) {
        drone.angleOY -= 2 * deltaTime;
    }

    //SimpleScene::GetSceneCamera()->SetPosition(glm::vec3(0, CAMERA_TO_DRONE_DIST_OY, CAMERA_TO_DRONE_DIST_OX));
    glm::quat cameraRotationOY = glm::angleAxis(drone.angleOY, glm::vec3(0, 1, 0));
    glm::quat cameraRotationOX = glm::angleAxis(-0.2f, glm::vec3(1, 0, 0));
    glm::quat cameraRotation = cameraRotationOY * cameraRotationOX;

    glm::vec3 cameraOffset = cameraRotation * glm::vec3(0, CAMERA_TO_DRONE_DIST_OY, CAMERA_TO_DRONE_DIST_OX);
    SimpleScene::GetSceneCamera()->SetPosition(drone.position + cameraOffset);
    SimpleScene::GetSceneCamera()->SetRotation(cameraRotation);
}

void Tema2::OnKeyPress(int key, int mods)
{
    /*if (key == GLFW_KEY_SPACE) {
    }*/
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

