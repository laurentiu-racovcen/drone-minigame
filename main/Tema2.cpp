#include "lab_m1/Tema2/main/Tema2.h"
#include "lab_m1/lab3/object2D.h"

#include <iostream>

using namespace std;
using namespace m1;

#define SKY_COLOR                0.6705, 0.788,  0.8529
#define TERRAIN_COLOR            0.455, 0.77, 0.463

#define DRONE_INITIAL_POSITION   0, 1, 0
#define CAMERA_TO_DRONE_DIST_OX  6.5
#define CAMERA_TO_DRONE_DIST_OY  1.25

#define TERRAIN_WIDTH            50
#define TERRAIN_LENGTH           50
#define MIN_TREE_SIZE            3
#define MAX_TREE_SIZE            10

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
    drone.angleOY = 0;

    // initialize drone coordinates
    drone.position = glm::vec3(DRONE_INITIAL_POSITION);

    // initialize the terrain
    unsigned int terrainM = TERRAIN_WIDTH;
    unsigned int terrainN = TERRAIN_LENGTH;
    glm::vec3 terrainColor = glm::vec3(TERRAIN_COLOR);
    terrain = Terrain(terrainM, terrainN, terrainColor);

    // add all objects meshes
	AddDroneMesh();
    AddDronePropellerMesh();
    AddTreeMesh(MAX_TREE_SIZE);
    AddTerrainMesh(&terrain);

    // add terrain shader
    Shader* shader = new Shader("TerrainShader");
    shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "terrain", "shaders", "TerrainVertexShader.glsl"), GL_VERTEX_SHADER);
    shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "terrain", "shaders", "TerrainFragmentShader.glsl"), GL_FRAGMENT_SHADER);
    shader->CreateAndLink();
    shaders[shader->GetName()] = shader;
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
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    /* --------------- Render the drone --------------- */

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

    RenderMesh(meshes["drone-propeller"], shaders["VertexColor"], propeller4Matrix);

    // update propellers angle
    if (drone.propellersAngle >= 2 * 3.14) {
        drone.propellersAngle = 0;
    } else {
        drone.propellersAngle += 10 * deltaTimeSeconds;
    }

    /* --------------- Render the terrain --------------- */

    glEnable(GL_DEPTH_TEST);
    glm::mat4 terrainMatrix = glm::mat4(1);
    terrainMatrix *= transform3D::Translate(-(float) terrain.n/2, 0, -(float) terrain.m/2);
    RenderTerrainMesh(meshes["terrain"], shaders["TerrainShader"], terrainMatrix);

    /* --------------- Render the trees --------------- */

    glm::mat4 treeMatrix = glm::mat4(1);
    //treeMatrix *= transform3D::Scale(0.5, 1, 0.5);
    treeMatrix *= transform3D::Translate(0, 0, 0);

    // render the drone
    RenderMesh(meshes["tree"], shaders["VertexColor"], treeMatrix);
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
