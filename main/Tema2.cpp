#include "lab_m1/Tema2/main/Tema2.h"
#include "lab_m1/Tema2/random/Random.h"

#include <iostream>

using namespace std;
using namespace m1;

#define SKY_COLOR                 0.6705, 0.788,  0.8529
#define TERRAIN_COLOR             0.455, 0.57, 0.293

#define DRONE_SIZE                1
#define DRONE_INITIAL_POSITION    0, 10, 0
#define DRONE_SPEED               12.0f
#define DRONE_PROPELLERS_SPEED    10
#define OBSTACLE_THROW_DISTANCE   30.0f

#define CAMERA_TO_DRONE_DIST_OX   6.5
#define CAMERA_TO_DRONE_DIST_OY   1.25

#define TERRAIN_WIDTH             200
#define TERRAIN_LENGTH            200
#define MIN_TREE_SCALE            0.55f
#define MAX_TREE_SCALE            2.0f
#define TREES_NUMBER              12

#define BUILDINGS_NUMBER          100
#define MIN_BUILDING_SCALE        5.5f
#define MAX_BUILDING_SCALE        20

#define MIN_PACKAGE_SCALE         1.0f
#define MAX_PACKAGE_SCALE         3.0f
#define PACKAGE_SPOT_RADIUS_SIZE  3.0f
#define PACKAGE_ARROW_SIZE        1.2f

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
    AddPackageMesh();
    AddPackageSrcLocationMesh();
    AddPackageDstLocationMesh();
    AddPackageLocationArrowMesh();
    AddTerrainMesh(&terrain);
    generateRandomBuildings(BUILDINGS_NUMBER, terrain.m, terrain.n);
    generateRandomTrees(TREES_NUMBER, terrain.m, terrain.n);

    // add terrain shader
    Shader* shader = new Shader("TerrainShader");
    shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "terrain", "shaders", "TerrainVertexShader.glsl"), GL_VERTEX_SHADER);
    shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "Tema2", "terrain", "shaders", "TerrainFragmentShader.glsl"), GL_FRAGMENT_SHADER);
    shader->CreateAndLink();
    shaders[shader->GetName()] = shader;

    // add text object
    // Create the text renderer object
    glm::ivec2 resolution = window->GetResolution();
    roundTextRenderer = new gfxc::TextRenderer(window->props.selfDir, resolution.x, resolution.y);
    roundTextRenderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, "Hack-Bold.ttf"), 20);

    playAgainTextRenderer = new gfxc::TextRenderer(window->props.selfDir, resolution.x, resolution.y);
    playAgainTextRenderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, "Hack-Bold.ttf"), 40);

    gameInterrupted = false;
    couldGeneratePackageLocation = true;

    // initialize the package
    generatePackageLocations();

    // initialize the score and package location radius
    score = 0;
    packageLocationRadius = PACKAGE_SPOT_RADIUS_SIZE;
    packageLocationRadiusStep = 0;

    // record start time
    startTime = std::chrono::high_resolution_clock::now();

    // record stop time
    stopTime = std::chrono::high_resolution_clock::now() + chrono::seconds(60);

    timeExpired = false;
    playAgainClicked = false;
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
Tree *Tema2::generateRandomTree(Tree* currentTree) {
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
        Tree* generatedTree = generateRandomTree(&currentTree);
        if (generatedTree != NULL) {
            trees.push_back(*generatedTree);
        }
    }
}

bool Tema2::packageIntersectsWithTree(glm::vec3 packagePosition, Tree obstacleTree) {
    float obstacleTreeRadius = obstacleTree.scale * LEAVES_DISK_SCALE;

    // compute the distance between the centers of the objects
    float dx = packagePosition.x - obstacleTree.position.x;
    float dz = packagePosition.z - obstacleTree.position.z;
    float centersDist = sqrt(dx * dx + dz * dz);

    if (centersDist > PACKAGE_SPOT_RADIUS_SIZE + obstacleTreeRadius) {
        return false;
    }

    return true;
}

bool Tema2::packageIntersectsWithBuilding(glm::vec3 packagePosition, Building obstacleBuilding) {
    /* check if the center of the tree is inside the building */
    if (packagePosition.x >= obstacleBuilding.position.x - obstacleBuilding.scale.x / 2 &&
        packagePosition.x <= obstacleBuilding.position.x + obstacleBuilding.scale.x / 2 &&
        packagePosition.z >= obstacleBuilding.position.z - obstacleBuilding.scale.z / 2 &&
        packagePosition.z <= obstacleBuilding.position.z + obstacleBuilding.scale.z / 2) {
        return true;
    }

    float closestX = std::max(obstacleBuilding.position.x - obstacleBuilding.scale.x / 2,
        std::min(packagePosition.x, obstacleBuilding.position.x + obstacleBuilding.scale.x / 2));
    float closestZ = std::max(obstacleBuilding.position.z - obstacleBuilding.scale.z / 2,
        std::min(packagePosition.z, obstacleBuilding.position.z + obstacleBuilding.scale.z / 2));

    // compute the distance from the tree's center to the closest point on the rectangle
    float dx = packagePosition.x - closestX;
    float dz = packagePosition.z - closestZ;
    float distanceToBuilding = sqrt(dx * dx + dz * dz);

    if (distanceToBuilding > PACKAGE_SPOT_RADIUS_SIZE) {
        return false;
    }

    return true;
}

bool Tema2::generateRandomPackage() {
    unsigned int tries = 20;
    while (tries != 0) {
        package.scale = MAX_PACKAGE_SCALE * Random::noise(glm::vec2(0.5, rand()));
        if (package.scale < MIN_PACKAGE_SCALE) {
            package.scale = MIN_PACKAGE_SCALE;
        }

        package.sourceLocation.x = terrain.n * Random::noise(glm::vec2(0.5, rand()));
        package.sourceLocation.z = terrain.m * Random::noise(glm::vec2(0.5, rand()));
        package.sourceLocation.y = MAX_TERRAIN_HEIGHT;

        bool intersectsObstacle = false;

        // check if current random-positioned source location intersects with a tree
        for (size_t i = 0; i < trees.size(); i++) {
            if (packageIntersectsWithTree(package.sourceLocation, trees[i])) {
                intersectsObstacle = true;
                continue;
            }
        }

        // check if current random-positioned destination location intersects with a building
        for (size_t i = 0; i < buildings.size(); i++) {
            if (packageIntersectsWithBuilding(package.sourceLocation, buildings[i])) {
                intersectsObstacle = true;
                continue;
            }
        }

        // a free source package spot has been found;
        // now find a free destination spot

        package.destinationLocation.x = terrain.n * Random::noise(glm::vec2(0.5, rand()));
        package.destinationLocation.z = terrain.m * Random::noise(glm::vec2(0.5, rand()));
        package.destinationLocation.y = MAX_TERRAIN_HEIGHT;

        // check if current random-positioned package intersects with a tree
        for (size_t i = 0; i < trees.size(); i++) {
            if (packageIntersectsWithTree(package.destinationLocation, trees[i])) {
                intersectsObstacle = true;
                continue;
            }
        }

        // check if current random-positioned tree intersects with a building
        for (size_t i = 0; i < buildings.size(); i++) {
            if (packageIntersectsWithBuilding(package.destinationLocation, buildings[i])) {
                intersectsObstacle = true;
                continue;
            }
        }

        if (!intersectsObstacle) {
            // a free spot has been found, set isInTransit to false
            package.isInTransit = false;
            return true;
        }

        tries--;
    }
    // a free spot has NOT been found for the package
    return false;
}

void Tema2::generatePackageLocations() {
    bool ret = generateRandomPackage();
    if (ret == false) {
        couldGeneratePackageLocation = false;
        gameInterrupted = true;
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

Building *Tema2::generateRandomBuilding(Building* currentBuilding) {
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
        Building* generatedBuilding = generateRandomBuilding(&currentBuilding);
        if (generatedBuilding != NULL) {
            buildings.push_back(*generatedBuilding);
        }
    }
}

float Tema2::getAngleBetweenPoints(glm::vec2 p1, glm::vec2 p2) {
    return atan2f((p2.y - p1.y), (p2.x - p1.x));
}

void Tema2::RenderDrone(float deltaTimeSeconds) {
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
    }
    else {
        drone.propellersAngle += DRONE_PROPELLERS_SPEED * deltaTimeSeconds;
    }
}

void Tema2::RenderTerrain() {
    glm::mat4 terrainMatrix = glm::mat4(1);
    terrainMatrix *= transform3D::Translate(-(float)terrain.n / 2, 0, -(float)terrain.m / 2);
    RenderTerrainMesh(meshes["terrain"], shaders["TerrainShader"], terrainMatrix);
}

void Tema2::RenderTrees() {
    for (size_t i = 0; i < trees.size(); i++) {
        // render current tree
        glm::mat4 treeMatrix = glm::mat4(1);
        treeMatrix *= transform3D::Translate(trees[i].position.x, trees[i].position.y, trees[i].position.z);
        treeMatrix *= transform3D::Translate(-(float)terrain.n / 2, 0, -(float)terrain.m / 2);
        treeMatrix *= transform3D::Scale(trees[i].scale, trees[i].scale, trees[i].scale);

        RenderMesh(meshes["tree"], shaders["VertexColor"], treeMatrix);
    }
}

void Tema2::RenderBuildings() {
    for (size_t i = 0; i < buildings.size(); i++) {
        // render current building
        glm::mat4 buildingMatrix = glm::mat4(1);
        buildingMatrix *= transform3D::Translate(buildings[i].position.x, buildings[i].position.y, buildings[i].position.z);
        buildingMatrix *= transform3D::Translate(-(float)terrain.n / 2, 0, -(float)terrain.m / 2);
        buildingMatrix *= transform3D::Scale(buildings[i].scale.x, buildings[i].scale.y, buildings[i].scale.z);

        RenderMesh(meshes["building"], shaders["VertexColor"], buildingMatrix);
    }
}

void Tema2::RenderPackage() {
    packageLocationRadius = PACKAGE_SPOT_RADIUS_SIZE - 0.5f * abs(sin(2*packageLocationRadiusStep));
    if (!package.isInTransit) {
        // render package at source location
        glm::mat4 packageMatrix = glm::mat4(1);
        packageMatrix *= transform3D::Translate(package.sourceLocation.x, package.sourceLocation.y, package.sourceLocation.z);
        packageMatrix *= transform3D::Translate(-(float)terrain.n / 2, 0, -(float)terrain.m / 2);
        packageMatrix *= transform3D::Scale(package.scale, package.scale, package.scale);

        RenderMesh(meshes["package"], shaders["VertexColor"], packageMatrix);

        // also render the source location disk
        RenderPackageSrcLocationDisk();
    } else {
        // render the package following the drone
        glm::mat4 packageMatrix = glm::mat4(1);
        // translate the package under the drone
        packageMatrix *= transform3D::Translate(0, -package.scale - 0.2f, 0);

        packageMatrix *= transform3D::Translate(drone.position.x, drone.position.y, drone.position.z);
        packageMatrix *= transform3D::RotateOY(drone.angleOY);
        packageMatrix *= transform3D::RotateOY(M_PI_4);
        packageMatrix *= transform3D::Scale(package.scale, package.scale, package.scale);

        RenderMesh(meshes["package"], shaders["VertexColor"], packageMatrix);

        // also render the destination location disk
        RenderPackageDstLocationDisk();
    }

    RenderPackageLocationArrow();
}

void Tema2::RenderPackageSrcLocationDisk() {
    // render package at source location
    glm::mat4 packageMatrix = glm::mat4(1);
    packageMatrix *= transform3D::Translate(package.sourceLocation.x, package.sourceLocation.y, package.sourceLocation.z);
    packageMatrix *= transform3D::Translate(-(float)terrain.n / 2, 0, -(float)terrain.m / 2);
    packageMatrix *= transform3D::Scale(packageLocationRadius, packageLocationRadius, packageLocationRadius);

    RenderMesh(meshes["package-source-location-disk"], shaders["VertexColor"], packageMatrix);
}

void Tema2::RenderPackageDstLocationDisk() {
    // render package at source location
    glm::mat4 packageMatrix = glm::mat4(1);
    packageMatrix *= transform3D::Translate(package.destinationLocation.x, package.destinationLocation.y, package.destinationLocation.z);
    packageMatrix *= transform3D::Translate(-(float)terrain.n / 2, 0, -(float)terrain.m / 2);
    packageMatrix *= transform3D::Scale(packageLocationRadius, packageLocationRadius, packageLocationRadius);

    RenderMesh(meshes["package-destination-location-disk"], shaders["VertexColor"], packageMatrix);
}

void Tema2::RenderPackageLocationArrow() {

    if (!package.isInTransit) {
        // render package at source location
        glm::mat4 packageMatrix = glm::mat4(1);
        packageMatrix *= transform3D::Translate(drone.position.x, drone.position.y + 0.3f, drone.position.z);
        packageMatrix *= transform3D::RotateOY(M_PI_2 + getAngleBetweenPoints(glm::vec2(drone.position.x, -drone.position.z), glm::vec2(package.sourceLocation.x  - (float)terrain.n / 2, -package.sourceLocation.z + (float)terrain.m / 2)));
        packageMatrix *= transform3D::Scale(PACKAGE_ARROW_SIZE, PACKAGE_ARROW_SIZE, PACKAGE_ARROW_SIZE);

        RenderMesh(meshes["package-location-arrow"], shaders["VertexColor"], packageMatrix);
    } else {
        // render package at source location
        glm::mat4 packageMatrix = glm::mat4(1);
        packageMatrix *= transform3D::Translate(drone.position.x, drone.position.y + 0.3f, drone.position.z);
        packageMatrix *= transform3D::RotateOY(M_PI_2 + getAngleBetweenPoints(glm::vec2(drone.position.x, -drone.position.z), glm::vec2(package.destinationLocation.x - (float)terrain.n / 2, -package.destinationLocation.z + (float)terrain.m / 2)));
        packageMatrix *= transform3D::Scale(PACKAGE_ARROW_SIZE, PACKAGE_ARROW_SIZE, PACKAGE_ARROW_SIZE);

        RenderMesh(meshes["package-location-arrow"], shaders["VertexColor"], packageMatrix);
    }
}

void Tema2::DrawHUD()
{
    const float kTopY = 35.f;
    const float kRowHeight = 25.f;

    int rowIndex = 0;
    std::string polygonModeText = "";

    polygonModeText = "solid";

    glm::ivec2 resolution = window->GetResolution();

    roundTextRenderer->RenderText("Score: " + to_string(score), resolution.x / 2 - 40.0f, kTopY + kRowHeight * rowIndex++, 1.0f, kTextColor);

    chrono::duration<double> remainingTime = stopTime - chrono::high_resolution_clock::now();

    ostringstream strTime;
    strTime << fixed << setprecision(2) << remainingTime.count();

    roundTextRenderer->RenderText("Remaining time: " + strTime.str() + " s", resolution.x / 2 - 125.0f, kTopY + kRowHeight * rowIndex++, 1.0f, kTextColor);

    if (!package.isInTransit) {
        roundTextRenderer->RenderText("Go to package source location!", resolution.x / 2 - 165.0f, kTopY + kRowHeight * rowIndex++, 1.0f, kTextColor);
    } else {
        roundTextRenderer->RenderText("Go to package destination location!", resolution.x / 2 - 195.0f, kTopY + kRowHeight * rowIndex++, 1.0f, kTextColor);
    }
}

void Tema2::showCouldNotGeneratePackageLocation() {
    const float kTopY = 410.f;
    const float kRowHeight = 55.f;

    int rowIndex = 0;
    std::string polygonModeText = "";

    polygonModeText = "solid";

    glm::ivec2 resolution = window->GetResolution();

    playAgainTextRenderer->RenderText("Could not generate new package location!", resolution.x / 2 - 435.0f, kTopY + kRowHeight * rowIndex++, 1.0f, kTextColor);
    playAgainTextRenderer->RenderText("Score: " + to_string(score), resolution.x / 2 - 70.0f, kTopY + kRowHeight * rowIndex++, 1.0f, kTextColor);
    playAgainTextRenderer->RenderText("Click to play again!", resolution.x / 2 - 215.0f, kTopY + kRowHeight * rowIndex++, 1.0f, kTextColor);
}

void Tema2::showPlayAgainText() {
    const float kTopY = 410.f;
    const float kRowHeight = 55.f;

    int rowIndex = 0;
    std::string polygonModeText = "";

    polygonModeText = "solid";

    glm::ivec2 resolution = window->GetResolution();

    playAgainTextRenderer->RenderText("Time expired!", resolution.x / 2 - 135.0f, kTopY + kRowHeight * rowIndex++, 1.0f, kTextColor);
    playAgainTextRenderer->RenderText("Score: " + to_string(score), resolution.x / 2 - 70.0f, kTopY + kRowHeight * rowIndex++, 1.0f, kTextColor);
    playAgainTextRenderer->RenderText("Click to play again!", resolution.x / 2 - 215.0f, kTopY + kRowHeight * rowIndex++, 1.0f, kTextColor);
}

void Tema2::Update(float deltaTimeSeconds)
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    chrono::duration<double> remainingTime = stopTime - chrono::high_resolution_clock::now();

    if (remainingTime.count() >= 0) {
        if (!couldGeneratePackageLocation) {
            showCouldNotGeneratePackageLocation();
            if (playAgainClicked) {
                // reset current map parameters
                trees.clear();
                buildings.clear();
                Init();
            }
        } else {
            RenderDrone(deltaTimeSeconds);
            RenderTerrain();
            RenderTrees();
            RenderBuildings();
            packageLocationRadiusStep += deltaTimeSeconds;
            RenderPackage();
            RenderPackageLocationArrow();
            DrawHUD();
        }
    } else {
        showPlayAgainText();
        if (playAgainClicked) {
            playAgainClicked = false;
            // reset current map obstacles
            trees.clear();
            buildings.clear();
            Init();
        }
    }
    
}

bool Tema2::DroneCollidesWithTerrain(glm::vec3 dronePosition) {
    int x = trunc(dronePosition.x + (float)terrain.n / 2);
    int z = trunc(dronePosition.z + (float)terrain.m / 2);
    tuple<int, int, int> coords = make_tuple(x, 0, z);

    // check if current tuple exists in the heights map
    if (terrain.verticesHeightsMap.find(coords) != terrain.verticesHeightsMap.end()) {
        // the key exists, check the corresponding height
        if (dronePosition.y <= terrain.verticesHeightsMap.at(coords)) {
            return true;
        }
    }

    return false;
}

bool Tema2::DroneCollidesWithMapLimits(glm::vec3 dronePosition) {

    if (dronePosition.x + (float)terrain.n / 2 <= 0) {
        return true;
    }

    if (dronePosition.x + (float)terrain.n / 2 >= terrain.n) {
        return true;
    }

    if (dronePosition.z + (float)terrain.m / 2 <= 0) {
        return true;
    }

    if (dronePosition.z + (float)terrain.m / 2 >= terrain.m) {
        return true;
    }

    return false;
}

bool Tema2::DroneCollidesWithABuilding(glm::vec3 dronePosition) {
    for (size_t i = 0; i < buildings.size(); i++) {
        // compare the position of the building's top corners with the drone's position

        /* don't compare if the height of the building is lower
         * than the height of the drone */
        if (buildings[i].position.y + buildings[i].scale.y >= drone.position.y - DRONE_SIZE * 0.1f) {
            // top left corner
            float x = buildings[i].position.x - buildings[i].scale.x / 2;
            float z = buildings[i].position.z - buildings[i].scale.z / 2;
            float y = buildings[i].position.y;
            glm::vec3 top_left = glm::vec3(x, y, z);

            // bottom right corner
            x = buildings[i].position.x + buildings[i].scale.x / 2;
            z = buildings[i].position.z + buildings[i].scale.z / 2;
            y = buildings[i].position.y;
            glm::vec3 bottom_right = glm::vec3(x, y, z);

            float droneX = drone.position.x + 1.0f * terrain.n / 2;
            float droneZ = drone.position.z + 1.0f * terrain.m / 2;

            if (top_left.x < droneX + DRONE_SIZE
                && top_left.z < droneZ + DRONE_SIZE
                && bottom_right.x > droneX - DRONE_SIZE
                && bottom_right.z > droneZ - DRONE_SIZE
                )
            {
                return true;
            }
        }
    }

    return false;
}

bool Tema2::disksIntersect(glm::vec3 disk1Position, glm::vec3 disk2Position, float disk1Radius, float disk2Radius) {

    float distDisksCenters = sqrt((disk1Position.x - disk2Position.x) * (disk1Position.x - disk2Position.x) +
                                  (disk1Position.z - disk2Position.z) * (disk1Position.z - disk2Position.z));

    if (distDisksCenters <= (disk1Radius + disk2Radius)) {
        return true;
    }
    return false;
}

bool Tema2::DroneCollidesWithATree(glm::vec3 dronePosition) {
    for (size_t i = 0; i < trees.size(); i++) {
        /* check if the drone collides with the tree's trunk */
        if (dronePosition.y <= trees[i].position.y + (TREE_TRUNK_HEIGHT) * trees[i].scale) {
            // the drone is below the current tree's trunk,
            // compare trunk and drone's positions
            float trunkDiskRadius = trees[i].scale * 0.5f;
            if (disksIntersect(dronePosition + 1.0f * terrain.n / 2, trees[i].position, trunkDiskRadius, 1.0f * DRONE_SIZE / 2)) {
                return true;
            }
        }

        /* check if the drone collides with the tree's leaves */
        if ((dronePosition.y <= trees[i].position.y + (TREE_TRUNK_HEIGHT + TREE_LEAVES_HEIGHT + 2 * LEAVES_DISTANCE) * trees[i].scale)
            && (dronePosition.y > trees[i].position.y + (TREE_TRUNK_HEIGHT)*trees[i].scale)) {

            // the drone is below the current tree's trunk,
            // compare trunk and drone's positions
            float trunkDiskRadius = trees[i].scale * 1.0f*LEAVES_DISK_SCALE/2;
            if (disksIntersect(dronePosition + 1.0f * terrain.n / 2, trees[i].position, trunkDiskRadius, 1.0f * DRONE_SIZE / 2)) {

                return true;
            }
        }
    }
    return false;
}

bool Tema2::DroneCollidesWithObstacles(glm::vec3 dronePosition) {
    if (DroneCollidesWithATree(dronePosition) || DroneCollidesWithABuilding(dronePosition)) {
        return true;
    }
    return false;
}

bool Tema2::DroneCollidesWithPackage() {
    // compare the position of the building's top corners with the drone's position

    /* don't compare if the height of the building is lower
        * than the height of the drone */
    if (package.sourceLocation.y + package.scale >= drone.position.y - DRONE_SIZE * 0.1f) {
        // top left corner
        float x = package.sourceLocation.x - package.scale / 2;
        float z = package.sourceLocation.z - package.scale / 2;
        float y = package.sourceLocation.y;
        glm::vec3 top_left = glm::vec3(x, y, z);

        // bottom right corner
        x = package.sourceLocation.x + package.scale / 2;
        z = package.sourceLocation.z + package.scale / 2;
        y = package.sourceLocation.y;
        glm::vec3 bottom_right = glm::vec3(x, y, z);

        float droneX = drone.position.x + 1.0f * terrain.n / 2;
        float droneZ = drone.position.z + 1.0f * terrain.m / 2;

        if (top_left.x < droneX + DRONE_SIZE
            && top_left.z < droneZ + DRONE_SIZE
            && bottom_right.x > droneX - DRONE_SIZE
            && bottom_right.z > droneZ - DRONE_SIZE
            )
        {
            return true;
        }
    }

    return false;
}

bool Tema2::DroneCollidesWithDestPackage() {
    if (disksIntersect(drone.position + 1.0f * terrain.n / 2, package.destinationLocation, PACKAGE_SPOT_RADIUS_SIZE, 1.0f * DRONE_SIZE / 2)) {
        return true;
    }
}

void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    if (!gameInterrupted) {

        // compute drone's forward and right directions
        glm::vec3 forward = glm::vec3(sin(drone.angleOY), 0, cos(drone.angleOY));
        glm::vec3 right = glm::vec3(cos(drone.angleOY), 0, -sin(drone.angleOY));

        // go left
        if (window->KeyHold(GLFW_KEY_A) == true) {
            // check if the drone does not collide with map's limits
            if (!DroneCollidesWithMapLimits(drone.position - right * DRONE_SPEED * deltaTime)
                && !DroneCollidesWithObstacles(drone.position - right * DRONE_SPEED * deltaTime))
            {
                drone.position -= right * DRONE_SPEED * deltaTime;
            }
            else if (DroneCollidesWithObstacles(drone.position - right * DRONE_SPEED * deltaTime)) {
                // move the drone in the opposite direction
                drone.position += glm::vec3(OBSTACLE_THROW_DISTANCE) * right * DRONE_SPEED * deltaTime;
            }
        }

        // go right
        if (window->KeyHold(GLFW_KEY_D) == true) {

            // check if the drone does not collide with map's limits
            if (!DroneCollidesWithMapLimits(drone.position + right * DRONE_SPEED * deltaTime)
                && !DroneCollidesWithObstacles(drone.position + right * DRONE_SPEED * deltaTime))
            {
                drone.position += right * DRONE_SPEED * deltaTime;
            }
            else if (DroneCollidesWithObstacles(drone.position + right * DRONE_SPEED * deltaTime)) {
                // move the drone in the opposite direction
                drone.position -= glm::vec3(OBSTACLE_THROW_DISTANCE) * right * DRONE_SPEED * deltaTime;
            }

        }

        // go front
        if (window->KeyHold(GLFW_KEY_W) == true) {

            // check if the drone does not collide with map's limits
            if (!DroneCollidesWithMapLimits(drone.position - forward * DRONE_SPEED * deltaTime)
                && !DroneCollidesWithObstacles(drone.position - forward * DRONE_SPEED * deltaTime))
            {
                drone.position -= forward * DRONE_SPEED * deltaTime;
            }
            else if (DroneCollidesWithObstacles(drone.position - forward * DRONE_SPEED * deltaTime)) {
                // move the drone in the opposite direction
                drone.position += glm::vec3(OBSTACLE_THROW_DISTANCE) * forward * DRONE_SPEED * deltaTime;
            }
        }

        // go back
        if (window->KeyHold(GLFW_KEY_S) == true) {

            // check if the drone does not collide with map's limits
            if (!DroneCollidesWithMapLimits(drone.position + forward * DRONE_SPEED * deltaTime)
                && !DroneCollidesWithObstacles(drone.position + forward * DRONE_SPEED * deltaTime))
            {
                drone.position += forward * DRONE_SPEED * deltaTime;
            }
            else if (DroneCollidesWithObstacles(drone.position + forward * DRONE_SPEED * deltaTime)) {
                // move the drone in the opposite direction
                drone.position -= glm::vec3(OBSTACLE_THROW_DISTANCE) * forward * DRONE_SPEED * deltaTime;
            }

        }

        // go up
        if (window->KeyHold(GLFW_KEY_Q) == true) {

            // check if the drone does not collide with an obstacle
            if (!DroneCollidesWithTerrain(glm::vec3(drone.position.x, drone.position.y - DRONE_SPEED * deltaTime, drone.position.z))
                && !DroneCollidesWithObstacles(glm::vec3(drone.position.x, drone.position.y - DRONE_SPEED * deltaTime, drone.position.z)))
            {
                // apply the position change to the drone
                drone.position.y -= DRONE_SPEED * deltaTime;
            }
            else if (DroneCollidesWithObstacles(glm::vec3(drone.position.x, drone.position.y - DRONE_SPEED * deltaTime, drone.position.z))) {
                // move the drone in the opposite direction
                drone.position.y += OBSTACLE_THROW_DISTANCE * DRONE_SPEED * deltaTime;
            }

        }

        // go down
        if (window->KeyHold(GLFW_KEY_E) == true) {

            // check if the drone does not collide with an obstacle
            if (!DroneCollidesWithObstacles(glm::vec3(drone.position.x, drone.position.y + DRONE_SPEED * deltaTime, drone.position.z))) {
                // apply the position change to the drone
                drone.position.y += DRONE_SPEED * deltaTime;
            }
            else if (DroneCollidesWithObstacles(glm::vec3(drone.position.x, drone.position.y + DRONE_SPEED * deltaTime, drone.position.z))) {
                // move the drone in the opposite direction
                drone.position.y -= OBSTACLE_THROW_DISTANCE * DRONE_SPEED * deltaTime;
            }

        }

        // rotate right
        if (window->KeyHold(GLFW_KEY_R) == true) {
            drone.angleOY += (DRONE_SPEED / 3) * deltaTime;
        }

        // rotate left
        if (window->KeyHold(GLFW_KEY_T) == true) {
            drone.angleOY -= (DRONE_SPEED / 3) * deltaTime;
        }

        // check if the drone collides with the source-located package
        if (package.isInTransit == false) {
            if (DroneCollidesWithPackage()) {
                package.isInTransit = true;
            }
        }
        else {
            // check if the drone has reached the destination of the package
            if (DroneCollidesWithDestPackage()) {
                score++;
                generatePackageLocations();
            }
        }

        glm::quat cameraRotationOY = glm::angleAxis(drone.angleOY, glm::vec3(0, 1, 0));
        glm::quat cameraRotationOX = glm::angleAxis(-0.2f, glm::vec3(1, 0, 0));
        glm::quat cameraRotation = cameraRotationOY * cameraRotationOX;

        glm::vec3 cameraOffset = cameraRotation * glm::vec3(0, CAMERA_TO_DRONE_DIST_OY, CAMERA_TO_DRONE_DIST_OX);
        SimpleScene::GetSceneCamera()->SetPosition(drone.position + cameraOffset);
        SimpleScene::GetSceneCamera()->SetRotation(cameraRotation);
    }
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
    if (button == GLFW_MOUSE_BUTTON_2) {
        playAgainClicked = true;
    }
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
