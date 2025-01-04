#include "lab_m1/Tema2/main/Tema2.h"
#include "lab_m1/Tema2/random/Random.h"

#include <iostream>

using namespace std;
using namespace m1;

#define SKY_COLOR                0.6705, 0.788,  0.8529
#define TERRAIN_COLOR            0.455, 0.77, 0.463

#define DRONE_SIZE               1
#define DRONE_INITIAL_POSITION   0, 1, 0
#define DRONE_PROPELLERS_SPEED   10

#define CAMERA_TO_DRONE_DIST_OX  6.5
#define CAMERA_TO_DRONE_DIST_OY  1.25

#define TERRAIN_WIDTH            100
#define TERRAIN_LENGTH           100
#define MIN_TREE_SCALE           0.15f
#define MAX_TREE_SCALE           1.0f
#define TREES_NUMBER             50

#define BUILDINGS_NUMBER         7
#define MIN_BUILDING_SCALE       4
#define MAX_BUILDING_SCALE       7

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
    AddTreeMesh();
    AddBuildingMesh();
    AddTerrainMesh(&terrain);
    generateRandomBuildings(BUILDINGS_NUMBER, terrain.m, terrain.n);
    generateRandomTrees(TREES_NUMBER, terrain.m, terrain.n);

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

bool Tema2::treeIntersectsWithOtherTree(Tree* currentTree, Tree obstacleTree) {
    float currentTreeRadius = currentTree->scale * LEAVES_DISK_SCALE;
    float obstacleTreeRadius = obstacleTree.scale * LEAVES_DISK_SCALE;

    // compute the distance between the centers of the trees
    float dx = currentTree->position.x - obstacleTree.position.x;
    float dz = currentTree->position.z - obstacleTree.position.z;
    float centersDist = sqrt(dx*dx + dz*dz);

    if (centersDist > currentTreeRadius + obstacleTreeRadius) {
        return false;
    }

    return true;
}

bool Tema2::treeIntersectsWithBuilding(Tree* currentTree, Building obstacleBuilding) {
    /* check if the center of the tree is inside the building */
    if (currentTree->position.x >= obstacleBuilding.position.x - obstacleBuilding.scale.x / 2 &&
        currentTree->position.x <= obstacleBuilding.position.x + obstacleBuilding.scale.x / 2 &&
        currentTree->position.z >= obstacleBuilding.position.z - obstacleBuilding.scale.z / 2 &&
        currentTree->position.z <= obstacleBuilding.position.z + obstacleBuilding.scale.z / 2) {
        return true;
    }

    float currentTreeRadius = currentTree->scale * LEAVES_DISK_SCALE;

    float closestX = std::max(obstacleBuilding.position.x - obstacleBuilding.scale.x / 2,
        std::min(currentTree->position.x, obstacleBuilding.position.x + obstacleBuilding.scale.x / 2));
    float closestZ = std::max(obstacleBuilding.position.z - obstacleBuilding.scale.z / 2,
        std::min(currentTree->position.z, obstacleBuilding.position.z + obstacleBuilding.scale.z / 2));

    // compute the distance from the tree's center to the closest point on the rectangle
    float dx = currentTree->position.x - closestX;
    float dz = currentTree->position.z - closestZ;
    float distanceToBuilding = sqrt(dx * dx + dz * dz);

    if (distanceToBuilding > currentTreeRadius) {
        return false;
    }

    return true;
}

// Function that searches for an obstacle-free position
Tree *Tema2::getRandomTree(Tree* currentTree) {
    unsigned int tries = 20;
    while (tries != 0) {
        currentTree->scale = MAX_TREE_SCALE * Random::noise(glm::vec2(0.5, rand()));
        if (currentTree->scale < MIN_TREE_SCALE) {
            currentTree->scale = MIN_TREE_SCALE;
        }

        currentTree->position.x = terrain.n * Random::noise(glm::vec2(0.5, rand()));
        currentTree->position.z = terrain.m * Random::noise(glm::vec2(0.5, rand()));
        currentTree->position.y = 0;

        bool intersectsObstacle = false;

        // check if current random-positioned tree intersects with other trees
        for (size_t i = 0; i < trees.size(); i++) {
            if (treeIntersectsWithOtherTree(currentTree, trees[i])) {
                intersectsObstacle = true;
                break;
            }
        }

        // check if current random-positioned tree intersects with other trees
        for (size_t i = 0; i < buildings.size(); i++) {
            if (treeIntersectsWithBuilding(currentTree, buildings[i])) {
                intersectsObstacle = true;
                break;
            }
        }

        if (!intersectsObstacle) {
            // a free spot has been found
            return currentTree;
        }
        tries--;
    }
    // a free spot has NOT been found for the current tree
    return NULL;
}



void Tema2::generateRandomTrees(unsigned int treesNum, unsigned int terrainWidth, unsigned int terrainLength) {
    for (unsigned int i = 0; i < treesNum; i++) {
        Tree currentTree;
        Tree* generatedTree = getRandomTree(&currentTree);
        if (generatedTree != NULL) {
            trees.push_back(*generatedTree);
        }
    }
}

bool Tema2::buildingIntersectsWithOtherBuilding(Building* currentBuilding, Building obstacleBuilding) {

    // check right1 side with left2 side
    float right1 = currentBuilding->position.x + 1.0f * currentBuilding->scale.x / 2;
    float left2 = obstacleBuilding.position.x - 1.0f * obstacleBuilding.scale.x / 2;

    if (right1 < left2) {
        return false;
    }

    // check top1 side with bottom2 side
    float top1 = currentBuilding->position.z + 1.0f * currentBuilding->scale.z / 2;
    float bottom2 = obstacleBuilding.position.z - 1.0f * obstacleBuilding.scale.z / 2;

    if (top1 < bottom2) {
        return false;
    }

    // check left1 side with right2 side
    float left1 = currentBuilding->position.x - 1.0f * currentBuilding->scale.x / 2;
    float right2 = obstacleBuilding.position.x + 1.0f * obstacleBuilding.scale.x / 2;

    if (left1 > right2) {
        return false;
    }

    // check bottom1 side with top2 side
    float bottom1 = currentBuilding->position.z - 1.0f * currentBuilding->scale.z / 2;
    float top2 = obstacleBuilding.position.z + 1.0f * obstacleBuilding.scale.z / 2;

    if (bottom1 > top2) {
        return false;
    }

    return true;
}

Building *Tema2::getRandomBuilding(Building* currentBuilding) {
    unsigned int tries = 20;
    while (tries != 0) {
        float randomScaleX = MAX_BUILDING_SCALE * Random::noise(glm::vec2(0.5, rand()));
        if (randomScaleX < MIN_BUILDING_SCALE) {
            randomScaleX = MIN_BUILDING_SCALE;
        }
        float randomScaleY = MAX_BUILDING_SCALE * Random::noise(glm::vec2(0.5, rand()));
        if (randomScaleY < MIN_BUILDING_SCALE) {
            randomScaleY = MIN_BUILDING_SCALE;
        }
        float randomScaleZ = MAX_BUILDING_SCALE * Random::noise(glm::vec2(0.5, rand()));
        if (randomScaleZ < MIN_BUILDING_SCALE) {
            randomScaleZ = MIN_BUILDING_SCALE;
        }

        currentBuilding->scale = glm::vec3(randomScaleX, randomScaleY, randomScaleZ);

        currentBuilding->position.x = terrain.n * Random::noise(glm::vec2(0.5, rand()));
        currentBuilding->position.z = terrain.m * Random::noise(glm::vec2(0.5, rand()));
        currentBuilding->position.y = 0;

        bool intersectsObstacle = false;

        // check if current random-positioned tree intersects with other trees
        for (size_t i = 0; i < buildings.size(); i++) {
            if (buildingIntersectsWithOtherBuilding(currentBuilding, buildings[i])) {
                intersectsObstacle = true;
                break;
            }
        }
        if (!intersectsObstacle) {
            // a free spot has been found
            return currentBuilding;
        }

        tries--;
    }

    return NULL;
}

void Tema2::generateRandomBuildings(unsigned int buildingsNum, unsigned int terrainWidth, unsigned int terrainLength) {
    for (unsigned int i = 0; i < buildingsNum; i++) {
        Building currentBuilding;
        Building* generatedBuilding = getRandomBuilding(&currentBuilding);
        if (generatedBuilding != NULL) {
            buildings.push_back(*generatedBuilding);
        }
    }
}

void Tema2::Update(float deltaTimeSeconds)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    /* --------------- Render the drone --------------- */

    glm::mat4 droneMatrix = glm::mat4(1);
    droneMatrix *= transform3D::Translate(drone.position.x, drone.position.y, drone.position.z);
    droneMatrix *= transform3D::RotateOY(drone.angleOY);
    droneMatrix *= transform3D::RotateOY(M_PI_4);
    droneMatrix *= transform3D::Scale(DRONE_SIZE, DRONE_SIZE, DRONE_SIZE);

    // render the drone
    RenderMesh(meshes["drone"], shaders["VertexColor"], droneMatrix);

    // render propeller 1
    glm::mat4 propeller1Matrix = glm::mat4(1);
    propeller1Matrix *= transform3D::Translate(drone.position.x, drone.position.y, drone.position.z);
    propeller1Matrix *= transform3D::RotateOY(drone.angleOY);
    propeller1Matrix *= transform3D::RotateOY(M_PI_4);
    propeller1Matrix *= transform3D::Translate(-DRONE_SIZE * 0.9f, DRONE_SIZE * 0.34f, 0);
    propeller1Matrix *= transform3D::RotateOY(drone.propellersAngle);
    propeller1Matrix *= transform3D::Scale(DRONE_SIZE, DRONE_SIZE, DRONE_SIZE);

    RenderMesh(meshes["drone-propeller"], shaders["VertexColor"], propeller1Matrix);

    // render propeller 2
    glm::mat4 propeller2Matrix = glm::mat4(1);
    propeller2Matrix *= transform3D::Translate(drone.position.x, drone.position.y, drone.position.z);
    propeller2Matrix *= transform3D::RotateOY(drone.angleOY);
    propeller2Matrix *= transform3D::RotateOY(M_PI_4);
    propeller2Matrix *= transform3D::Translate(DRONE_SIZE * 0.9f, DRONE_SIZE * 0.34f, 0);
    propeller2Matrix *= transform3D::RotateOY(drone.propellersAngle);
    propeller2Matrix *= transform3D::Scale(DRONE_SIZE, DRONE_SIZE, DRONE_SIZE);

    RenderMesh(meshes["drone-propeller"], shaders["VertexColor"], propeller2Matrix);

    // render propeller 3
    glm::mat4 propeller3Matrix = glm::mat4(1);
    propeller3Matrix *= transform3D::Translate(drone.position.x, drone.position.y, drone.position.z);
    propeller3Matrix *= transform3D::RotateOY(drone.angleOY);
    propeller3Matrix *= transform3D::RotateOY(M_PI_4);
    propeller3Matrix *= transform3D::Translate(0, DRONE_SIZE * 0.34f, -DRONE_SIZE * 0.9f);
    propeller3Matrix *= transform3D::RotateOY(drone.propellersAngle);
    propeller3Matrix *= transform3D::Scale(DRONE_SIZE, DRONE_SIZE, DRONE_SIZE);

    RenderMesh(meshes["drone-propeller"], shaders["VertexColor"], propeller3Matrix);

    // render propeller 4
    glm::mat4 propeller4Matrix = glm::mat4(1);
    propeller4Matrix *= transform3D::Translate(drone.position.x, drone.position.y, drone.position.z);
    propeller4Matrix *= transform3D::RotateOY(drone.angleOY);
    propeller4Matrix *= transform3D::RotateOY(M_PI_4);
    propeller4Matrix *= transform3D::Translate(0, DRONE_SIZE * 0.34f, DRONE_SIZE * 0.9f);
    propeller4Matrix *= transform3D::RotateOY(drone.propellersAngle);
    propeller4Matrix *= transform3D::Scale(DRONE_SIZE, DRONE_SIZE, DRONE_SIZE);

    RenderMesh(meshes["drone-propeller"], shaders["VertexColor"], propeller4Matrix);

    // update propellers angle
    if (drone.propellersAngle >= 2 * M_PI) {
        drone.propellersAngle = 0;
    } else {
        drone.propellersAngle += DRONE_PROPELLERS_SPEED * deltaTimeSeconds;
    }

    /* --------------- Render the terrain --------------- */

    glEnable(GL_DEPTH_TEST);
    glm::mat4 terrainMatrix = glm::mat4(1);
    terrainMatrix *= transform3D::Translate(-(float) terrain.n/2, 0, -(float) terrain.m/2);
    RenderTerrainMesh(meshes["terrain"], shaders["TerrainShader"], terrainMatrix);

    /* --------------- Render the trees --------------- */

    for (size_t i = 0; i < trees.size(); i++) {
        // render current tree
        glm::mat4 treeMatrix = glm::mat4(1);
        treeMatrix *= transform3D::Translate(trees[i].position.x, trees[i].position.y, trees[i].position.z);
        treeMatrix *= transform3D::Translate(-(float)terrain.n / 2, 0, -(float)terrain.m / 2);
        treeMatrix *= transform3D::Scale(trees[i].scale, trees[i].scale, trees[i].scale);

        RenderMesh(meshes["tree"], shaders["VertexColor"], treeMatrix);
    }

    /* --------------- Render the buildings --------------- */

    for (size_t i = 0; i < buildings.size(); i++) {
        // render current building
        glm::mat4 buildingMatrix = glm::mat4(1);
        buildingMatrix *= transform3D::Translate(buildings[i].position.x, buildings[i].position.y, buildings[i].position.z);
        buildingMatrix *= transform3D::Translate(-(float)terrain.n / 2, 0, -(float)terrain.m / 2);
        buildingMatrix *= transform3D::Scale(buildings[i].scale.x, buildings[i].scale.y, buildings[i].scale.z);

        RenderMesh(meshes["building"], shaders["VertexColor"], buildingMatrix);
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
