#include "game/main/DroneMinigame.h"

#include <iostream>

using namespace std;
using namespace m1;

#define SKY_COLOR                   0.6705, 0.788,  0.8529
#define TERRAIN_COLOR               0.455, 0.57, 0.293

#define DRONE_SIZE                  1
#define DRONE_INITIAL_POSITION      0, 10, 0
#define DRONE_SPEED                 12.0f
#define DRONE_PROPELLERS_SPEED      10
#define OBSTACLE_THROW_DISTANCE     30.0f
#define MOUSE_ROTATION_SENSITIVITY  0.25f

#define CAMERA_TO_DRONE_DIST_OX     6.5
#define CAMERA_TO_DRONE_DIST_OY     1.25

#define TERRAIN_WIDTH               200
#define TERRAIN_LENGTH              200
#define MIN_TREE_SCALE              0.55f
#define MAX_TREE_SCALE              2.0f
#define TREES_NUMBER                50

#define BUILDINGS_NUMBER            20
#define MIN_BUILDING_SCALE          5.5f
#define MAX_BUILDING_SCALE          20

#define MIN_PACKAGE_SCALE           1.0f
#define MAX_PACKAGE_SCALE           3.0f
#define PACKAGE_SPOT_RADIUS_SIZE    3.0f
#define PACKAGE_ARROW_SIZE          1.2f

#define LOCATION_SEARCH_TRIES_NUM   20

DroneMinigame::DroneMinigame()
{
}

DroneMinigame::~DroneMinigame()
{
}

void DroneMinigame::Init()
{
    gameStarted = false;

    // initialize propellers angle
    drone.propellersAngle = 0;
    drone.angleOY = 0;

    // initialize the terrain
    unsigned int terrainM = TERRAIN_WIDTH;
    unsigned int terrainN = TERRAIN_LENGTH;
    glm::vec3 terrainColor = glm::vec3(TERRAIN_COLOR);
    terrain = Terrain(terrainM, terrainN, terrainColor);

    // add all objects meshes
    AddAllMeshes();

    // add terrain shader
    Shader* shader = new Shader("TerrainShader");
    shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::GAME, "terrain", "shaders", "TerrainVertexShader.glsl"), GL_VERTEX_SHADER);
    shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::GAME, "terrain", "shaders", "TerrainFragmentShader.glsl"), GL_FRAGMENT_SHADER);
    shader->CreateAndLink();
    shaders[shader->GetName()] = shader;

    // add text object
    // create the text renderer objects
    glm::ivec2 resolution = window->GetResolution();

    titleTextRenderer = new gfxc::TextRenderer(window->props.selfDir, resolution.x, resolution.y);
    titleTextRenderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, "Hack-Bold.ttf"), 100);

    roundTextRenderer = new gfxc::TextRenderer(window->props.selfDir, resolution.x, resolution.y);
    roundTextRenderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, "Hack-Bold.ttf"), 20);

    playAgainTextRenderer = new gfxc::TextRenderer(window->props.selfDir, resolution.x, resolution.y);
    playAgainTextRenderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, "Hack-Bold.ttf"), 40);

    subtextRenderer = new gfxc::TextRenderer(window->props.selfDir, resolution.x, resolution.y);
    subtextRenderer->Load(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::FONTS, "Hack-Bold.ttf"), 20);

    window->ShowPointer();

    StartNewRound();
}

void DroneMinigame::AddAllMeshes() {
    AddDroneMesh();
    AddDronePropellerMesh();
    AddTreeMesh();
    AddBuildingMesh();
    AddPackageMesh();
    AddPackageSrcLocationMesh();
    AddPackageDstLocationMesh();
    AddPackageLocationArrowMesh();
    AddDroneLocationMinimapArrowMesh();
    AddTerrainMesh(&terrain);
}

void DroneMinigame::StartNewRound() {
    generateRandomBuildings(BUILDINGS_NUMBER, terrain.m, terrain.n);
    generateRandomTrees(TREES_NUMBER, terrain.m, terrain.n);

    // initialize drone coordinates
    drone.position = getDroneInitialPosition();

    gameInterrupted = false;
    couldGeneratePackageLocation = true;

    // initialize the package and package's source and destination locations
    generatePackageLocations();

    // initialize the score and package location radius
    score = 0;
    packageLocationRadius = PACKAGE_SPOT_RADIUS_SIZE;
    packageLocationRadiusStep = 0; // for location disk animation

    // record start time
    startTime = std::chrono::high_resolution_clock::now();

    // record stop time
    stopTime = std::chrono::high_resolution_clock::now() + chrono::seconds(60);

    // initialize check variables
    timeExpired = false;
    playAgainClicked = false;

    glm::ivec2 resolution = window->GetResolution();

    // initialize the minimap
    minimap = Minimap(0, 0, resolution.x, resolution.y);
}

void DroneMinigame::FrameStart()
{
}

void DroneMinigame::FrameEnd()
{
}

/* Function that returns the initial position of the drone
 * based on the tallest obstacle */
glm::vec3 DroneMinigame::getDroneInitialPosition() {
    float maxHeight = 0;
    for (size_t i = 0; i < trees.size(); i++) {
        if (trees[i].position.y + (TREE_TRUNK_HEIGHT + TREE_LEAVES_HEIGHT + 2 * LEAVES_DISTANCE) * trees[i].scale > maxHeight) {
            maxHeight = trees[i].position.y + (TREE_TRUNK_HEIGHT + TREE_LEAVES_HEIGHT + 2 * LEAVES_DISTANCE) * trees[i].scale;
        }
    }

    for (size_t i = 0; i < buildings.size(); i++) {
        if (buildings[i].position.y + buildings[i].scale.y > maxHeight) {
            maxHeight = buildings[i].position.y + buildings[i].scale.y;
        }
    }
    return glm::vec3(0, maxHeight + DRONE_SIZE * 0.5f, 0);
}

bool DroneMinigame::treeIntersectsWithOtherTree(Tree* currentTree, Tree obstacleTree) {
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

bool DroneMinigame::treeIntersectsWithBuilding(Tree* currentTree, Building obstacleBuilding) {
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

/* Function that searches for an obstacle-free position
 * in order to set the position of a tree */
Tree *DroneMinigame::generateRandomTree(Tree* currentTree) {
    unsigned int tries = LOCATION_SEARCH_TRIES_NUM;
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

void DroneMinigame::generateRandomTrees(unsigned int treesNum, unsigned int terrainWidth, unsigned int terrainLength) {
    for (unsigned int i = 0; i < treesNum; i++) {
        Tree currentTree;
        Tree* generatedTree = generateRandomTree(&currentTree);
        if (generatedTree != NULL) {
            trees.push_back(*generatedTree);
        }
    }
}

bool DroneMinigame::packageIntersectsWithTree(glm::vec3 packagePosition, Tree obstacleTree) {
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

bool DroneMinigame::packageIntersectsWithBuilding(glm::vec3 packagePosition, Building obstacleBuilding) {
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

bool DroneMinigame::generateRandomPackage() {
    unsigned int tries = LOCATION_SEARCH_TRIES_NUM;
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

void DroneMinigame::generatePackageLocations() {
    bool ret = generateRandomPackage();
    if (ret == false) {
        couldGeneratePackageLocation = false;
        gameInterrupted = true;
    }
}

bool DroneMinigame::buildingIntersectsWithOtherBuilding(Building* currentBuilding, Building obstacleBuilding) {
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

Building *DroneMinigame::generateRandomBuilding(Building* currentBuilding) {
    unsigned int tries = LOCATION_SEARCH_TRIES_NUM;
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

void DroneMinigame::generateRandomBuildings(unsigned int buildingsNum, unsigned int terrainWidth, unsigned int terrainLength) {
    for (unsigned int i = 0; i < buildingsNum; i++) {
        Building currentBuilding;
        Building* generatedBuilding = generateRandomBuilding(&currentBuilding);
        if (generatedBuilding != NULL) {
            buildings.push_back(*generatedBuilding);
        }
    }
}

float DroneMinigame::getAngleBetweenPoints(glm::vec2 p1, glm::vec2 p2) {
    return atan2f((p2.y - p1.y), (p2.x - p1.x));
}

void DroneMinigame::RenderDrone(float deltaTimeSeconds) {
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

void DroneMinigame::RenderTerrain() {
    glm::mat4 terrainMatrix = glm::mat4(1);
    terrainMatrix *= transform3D::Translate(-(float)terrain.n / 2, 0, -(float)terrain.m / 2);
    RenderTerrainMesh(meshes["terrain"], shaders["TerrainShader"], terrainMatrix);
}

void DroneMinigame::RenderTrees() {
    for (size_t i = 0; i < trees.size(); i++) {
        // render current tree
        glm::mat4 treeMatrix = glm::mat4(1);
        treeMatrix *= transform3D::Translate(trees[i].position.x, trees[i].position.y, trees[i].position.z);
        treeMatrix *= transform3D::Translate(-(float)terrain.n / 2, 0, -(float)terrain.m / 2);
        treeMatrix *= transform3D::Scale(trees[i].scale, trees[i].scale, trees[i].scale);

        RenderMesh(meshes["tree"], shaders["VertexColor"], treeMatrix);
    }
}

void DroneMinigame::RenderBuildings() {
    for (size_t i = 0; i < buildings.size(); i++) {
        // render current building
        glm::mat4 buildingMatrix = glm::mat4(1);
        buildingMatrix *= transform3D::Translate(buildings[i].position.x, buildings[i].position.y, buildings[i].position.z);
        buildingMatrix *= transform3D::Translate(-(float)terrain.n / 2, 0, -(float)terrain.m / 2);
        buildingMatrix *= transform3D::Scale(buildings[i].scale.x, buildings[i].scale.y, buildings[i].scale.z);

        RenderMesh(meshes["building"], shaders["VertexColor"], buildingMatrix);
    }
}

void DroneMinigame::RenderPackage() {
    packageLocationRadius = PACKAGE_SPOT_RADIUS_SIZE - 0.5f * abs(sin(2*packageLocationRadiusStep));
    if (!package.isInTransit) {
        // render package at source location
        glm::mat4 packageMatrix = glm::mat4(1);
        packageMatrix *= transform3D::Translate(package.sourceLocation.x, package.sourceLocation.y, package.sourceLocation.z);
        packageMatrix *= transform3D::Translate(-(float)terrain.n / 2, 0, -(float)terrain.m / 2);
        packageMatrix *= transform3D::Scale(package.scale, package.scale, package.scale);

        RenderMesh(meshes["package"], shaders["VertexColor"], packageMatrix);
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
    }
}

void DroneMinigame::RenderLocation() {
    if (!package.isInTransit) {
        // render the source location disk
        RenderPackageSrcLocationDisk();
    } else {
        // render the destination location disk
        RenderPackageDstLocationDisk();
    }
}

void DroneMinigame::RenderLocationOnMinimap() {
    if (!package.isInTransit) {
        // render the source location disk
        RenderPackageSrcLocationDiskMinimap();
    }
    else {
        // render the destination location disk
        RenderPackageDstLocationDiskMinimap();
    }
}

void DroneMinigame::RenderPackageSrcLocationDisk() {
    // render source location disk
    glm::mat4 diskMatrix = glm::mat4(1);
    diskMatrix *= transform3D::Translate(package.sourceLocation.x, package.sourceLocation.y, package.sourceLocation.z);
    diskMatrix *= transform3D::Translate(-(float)terrain.n / 2, 0, -(float)terrain.m / 2);
    diskMatrix *= transform3D::Scale(packageLocationRadius, packageLocationRadius, packageLocationRadius);

    RenderMesh(meshes["package-source-location-disk"], shaders["VertexColor"], diskMatrix);
}

void DroneMinigame::RenderPackageSrcLocationDiskMinimap() {
    // render source location disk for minimap
    glm::mat4 diskMatrix = glm::mat4(1);
    diskMatrix *= transform3D::Translate(package.sourceLocation.x, package.sourceLocation.y, package.sourceLocation.z);
    diskMatrix *= transform3D::Translate(-(float)terrain.n / 2, 0, -(float)terrain.m / 2);
    diskMatrix *= transform3D::Scale(3*packageLocationRadius, 3*packageLocationRadius, 3*packageLocationRadius);

    RenderMesh(meshes["package-source-location-disk"], shaders["VertexColor"], diskMatrix);
}


void DroneMinigame::RenderPackageDstLocationDisk() {
    // render destination location disk
    glm::mat4 diskMatrix = glm::mat4(1);
    diskMatrix *= transform3D::Translate(package.destinationLocation.x, package.destinationLocation.y, package.destinationLocation.z);
    diskMatrix *= transform3D::Translate(-(float)terrain.n / 2, 0, -(float)terrain.m / 2);
    diskMatrix *= transform3D::Scale(packageLocationRadius, packageLocationRadius, packageLocationRadius);

    RenderMesh(meshes["package-destination-location-disk"], shaders["VertexColor"], diskMatrix);
}

void DroneMinigame::RenderPackageDstLocationDiskMinimap() {
    // render destination location disk for minimap
    glm::mat4 diskMatrix = glm::mat4(1);
    diskMatrix *= transform3D::Translate(package.destinationLocation.x, package.destinationLocation.y, package.destinationLocation.z);
    diskMatrix *= transform3D::Translate(-(float)terrain.n / 2, 0, -(float)terrain.m / 2);
    diskMatrix *= transform3D::Scale(3*packageLocationRadius, 3*packageLocationRadius, 3*packageLocationRadius);

    RenderMesh(meshes["package-destination-location-disk"], shaders["VertexColor"], diskMatrix);
}

void DroneMinigame::RenderPackageLocationArrow() {

    if (!package.isInTransit) {
        // render package at source location
        glm::mat4 arrowMatrix = glm::mat4(1);
        arrowMatrix *= transform3D::Translate(drone.position.x, drone.position.y + 0.3f, drone.position.z);
        arrowMatrix *= transform3D::RotateOY(M_PI_2 + getAngleBetweenPoints(glm::vec2(drone.position.x, -drone.position.z), glm::vec2(package.sourceLocation.x - (float)terrain.n / 2, -package.sourceLocation.z + (float)terrain.m / 2)));
        arrowMatrix *= transform3D::Scale(PACKAGE_ARROW_SIZE, PACKAGE_ARROW_SIZE, PACKAGE_ARROW_SIZE);

        RenderMesh(meshes["package-location-arrow"], shaders["VertexColor"], arrowMatrix);
    }
    else {
        // render package at destination location
        glm::mat4 arrowMatrix = glm::mat4(1);
        arrowMatrix *= transform3D::Translate(drone.position.x, drone.position.y + 0.3f, drone.position.z);
        arrowMatrix *= transform3D::RotateOY(M_PI_2 + getAngleBetweenPoints(glm::vec2(drone.position.x, -drone.position.z), glm::vec2(package.destinationLocation.x - (float)terrain.n / 2, -package.destinationLocation.z + (float)terrain.m / 2)));
        arrowMatrix *= transform3D::Scale(PACKAGE_ARROW_SIZE, PACKAGE_ARROW_SIZE, PACKAGE_ARROW_SIZE);

        RenderMesh(meshes["package-location-arrow"], shaders["VertexColor"], arrowMatrix);
    }
}

void DroneMinigame::RenderDroneLocationMinimapArrow() {
    // render arrow pointing to drone direction
    glm::mat4 arrowMatrix = glm::mat4(1);
    arrowMatrix *= transform3D::Translate(drone.position.x, drone.position.y + 0.3f, drone.position.z);
    arrowMatrix *= transform3D::RotateOY(M_PI + drone.angleOY);
    arrowMatrix *= transform3D::Scale(PACKAGE_ARROW_SIZE*20, PACKAGE_ARROW_SIZE*20, PACKAGE_ARROW_SIZE*20);

    RenderMesh(meshes["drone-location-minimap-arrow"], shaders["VertexColor"], arrowMatrix);
}

void DroneMinigame::DrawHUD()
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

void DroneMinigame::showCouldNotGeneratePackageLocation() {
    const float kTopY = 410.f;
    const float kRowHeight = 55.f;

    int rowIndex = 0;
    std::string polygonModeText = "";
    polygonModeText = "solid";

    glm::ivec2 resolution = window->GetResolution();

    playAgainTextRenderer->RenderText("Error: Could not generate new package location!", resolution.x / 2 - 545.0f, kTopY + kRowHeight * rowIndex++, 1.0f, kTextColor);
    playAgainTextRenderer->RenderText("Score: " + to_string(score), resolution.x / 2 - 70.0f, kTopY + kRowHeight * rowIndex++, 1.0f, kTextColor);
    playAgainTextRenderer->RenderText("Click to play again!", resolution.x / 2 - 215.0f, kTopY + kRowHeight * rowIndex++, 1.0f, kTextColor);
}

void DroneMinigame::showPlayAgainText() {
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

void DroneMinigame::RenderGameIntro() {
    const float kTopY = 410.f;
    const float kRowHeight = 155.f;

    int rowIndex = 0;
    std::string polygonModeText = "";
    polygonModeText = "solid";

    glm::ivec2 resolution = window->GetResolution();

    titleTextRenderer->RenderText("Drone Delivery Game", resolution.x / 2 - 535.0f, kTopY + kRowHeight * rowIndex++, 1.0f, kTextColor);
    playAgainTextRenderer->RenderText("Click to play!", resolution.x / 2 - 175.0f, kTopY + kRowHeight * rowIndex++, 1.0f, kTextColor);
    subtextRenderer->RenderText("(c) Racovcen Laurentiu", resolution.x / 2 - 135.0f, 200 + kTopY + kRowHeight * rowIndex++, 1.0f, kTextColor);
}

void DroneMinigame::Update(float deltaTimeSeconds)
{
    /* ---------- Draw main viewport ---------- */

    // clears the color buffer (using the previously set color) and depth buffer
    glClearColor(SKY_COLOR, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();

    // sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    chrono::duration<double> remainingTime = stopTime - chrono::high_resolution_clock::now();

    if (!gameStarted) {
        // show game intro
        if (cursorIsHidden) {
            cursorIsHidden = false;
            window->ShowPointer();
        }
        RenderGameIntro();
    } else {
        if (!couldGeneratePackageLocation) {
            showCouldNotGeneratePackageLocation();
            if (playAgainClicked) {
                playAgainClicked = false;
                // clear current map obstacles
                trees.clear();
                buildings.clear();
                StartNewRound();
            }
        }
        else if (remainingTime.count() >= 0) {
            if (!cursorIsHidden) {
                cursorIsHidden = true;
                window->DisablePointer();
            }

            RenderDrone(deltaTimeSeconds);
            RenderTerrain();
            RenderTrees();
            RenderBuildings();
            RenderPackage();
            RenderLocation();
            RenderPackageLocationArrow();
            packageLocationRadiusStep += deltaTimeSeconds;
            DrawHUD();

            /* ---------- Draw minimap viewport ---------- */

            // rotate camera on OX and reset OY angle
            float newOYAngle = 0;
            glm::quat cameraRotationOY = glm::angleAxis(newOYAngle, glm::vec3(0, 1, 0));
            glm::quat cameraRotationOX = glm::angleAxis(-1.5f, glm::vec3(1, 0, 0));
            glm::quat cameraRotation = cameraRotationOY * cameraRotationOX;
            SimpleScene::GetSceneCamera()->SetRotation(cameraRotation);

            glClear(GL_DEPTH_BUFFER_BIT);
            glViewport(1500, 50, resolution.x / 7.0f, resolution.x / 7.0f);

            // set up the orthographic projection for the minimap
            float left = drone.position.x - 1.0f * TERRAIN_WIDTH / 2;
            float right = drone.position.x + 1.0f * TERRAIN_WIDTH / 2;
            float bottom = drone.position.z - 1.0f * TERRAIN_WIDTH / 2;
            float top = drone.position.z + 1.0f * TERRAIN_WIDTH / 2;
            float near = -1000.0f;
            float far = 1000.0f;

            // update the minimap's camera projection matrix to be orthographic
            SimpleScene::GetSceneCamera()->SetOrthographic(left, right, bottom, top, near, far);

            /* enable depth writing for the arrow,
             * in order for the arrow to show over other objects */
            glDepthMask(GL_FALSE);
            RenderTerrain();
            RenderTrees();
            RenderBuildings();
            RenderLocation();
            RenderLocationOnMinimap();
            RenderDroneLocationMinimapArrow();
            glDepthMask(GL_TRUE);

            // reset camera to perspective
            SimpleScene::GetSceneCamera()->SetPerspective(90, 16.0f / 9, 0.01, 1000);
        } else {
            if (cursorIsHidden) {
                window->ShowPointer();
                cursorIsHidden = false;
            }

            timeExpired = true;
            showPlayAgainText();
            // clear current map obstacles
            trees.clear();
            buildings.clear();
            if (playAgainClicked) {
                playAgainClicked = false;
                StartNewRound();
            }
        }
    }
    
}

bool DroneMinigame::DroneCollidesWithTerrain(glm::vec3 dronePosition) {
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

bool DroneMinigame::DroneCollidesWithMapLimits(glm::vec3 dronePosition) {

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

bool DroneMinigame::DroneCollidesWithABuilding(glm::vec3 dronePosition) {
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

bool DroneMinigame::disksIntersect(glm::vec3 disk1Position, glm::vec3 disk2Position, float disk1Radius, float disk2Radius) {

    float distDisksCenters = sqrt((disk1Position.x - disk2Position.x) * (disk1Position.x - disk2Position.x) +
                                  (disk1Position.z - disk2Position.z) * (disk1Position.z - disk2Position.z));

    if (distDisksCenters <= (disk1Radius + disk2Radius)) {
        return true;
    }
    return false;
}

bool DroneMinigame::DroneCollidesWithATree(glm::vec3 dronePosition) {
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
            // the drone is below the current tree's leaves,
            // compare leaves and drone's positions
            float trunkDiskRadius = trees[i].scale * 1.0f*LEAVES_DISK_SCALE/2;
            if (disksIntersect(dronePosition + 1.0f * terrain.n / 2, trees[i].position, trunkDiskRadius, 1.0f * DRONE_SIZE / 2)) {
                return true;
            }
        }
    }
    return false;
}

bool DroneMinigame::DroneCollidesWithObstacles(glm::vec3 dronePosition) {
    if (DroneCollidesWithATree(dronePosition) || DroneCollidesWithABuilding(dronePosition)) {
        return true;
    }
    return false;
}

bool DroneMinigame::DroneCollidesWithPackage() {
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

bool DroneMinigame::DroneCollidesWithDestPackage() {
    if (disksIntersect(drone.position + 1.0f * terrain.n / 2, package.destinationLocation, PACKAGE_SPOT_RADIUS_SIZE, 1.0f * DRONE_SIZE / 2)) {
        return true;
    }
}

void DroneMinigame::OnInputUpdate(float deltaTime, int mods)
{
    if (!gameInterrupted) {
        // compute drone's forward and right directions
        glm::vec3 forward = glm::vec3(sin(drone.angleOY), 0, cos(drone.angleOY));
        glm::vec3 right = glm::vec3(cos(drone.angleOY), 0, -sin(drone.angleOY));

        // go left
        if (window->KeyHold(GLFW_KEY_A) == true) {
            // check if the drone does not collide with map's limits or obstacles
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

            // check if the drone does not collide with map's limits or obstacles
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

            // check if the drone does not collide with map's limits or obstacles
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

            // check if the drone does not collide with map's limits or obstacles
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

        /* rotate the drone according to mouse movements (with configurable sensitivity) */

        static double lastMouseX = 0;
        static double lastMouseY = 0;
        static bool firstMouse = true;
        static double smoothedDeltaX = 0;
        static double smoothedDeltaY = 0;

        double mouseX, mouseY;
        mouseX = window->props.cursorPos.x;
        mouseY = window->props.cursorPos.y;

        if (firstMouse) {
            lastMouseX = mouseX;
            lastMouseY = mouseY;
            firstMouse = false;
        }

        double deltaMouseX = mouseX - lastMouseX;
        double deltaMouseY = mouseY - lastMouseY;
        lastMouseX = mouseX;
        lastMouseY = mouseY;

        // apply exponential moving average smoothing
        float smoothingFactor = 0.3f;
        smoothedDeltaX = smoothingFactor * deltaMouseX + (1.0f - smoothingFactor) * smoothedDeltaX;
        smoothedDeltaY = smoothingFactor * deltaMouseY + (1.0f - smoothingFactor) * smoothedDeltaY;

        // clamp the smoothed delta to prevent teleporting
        const double MAX_MOUSE_DELTA = 10.0f; // clamp threshhold
        smoothedDeltaY = glm::clamp(smoothedDeltaY, -MAX_MOUSE_DELTA, MAX_MOUSE_DELTA);

        float rotationSpeed = MOUSE_ROTATION_SENSITIVITY * deltaTime;
        float verticalSpeed = 0.25f * DRONE_SPEED * deltaTime;

        // horizontal rotation (OY) - mouse X movement
        if (abs(smoothedDeltaX) > 0.001) {
            drone.angleOY -= smoothedDeltaX * rotationSpeed;
        }

        // vertical rotation (OX) - mouse Y movement  
        if (abs(smoothedDeltaY) > 0.001) {
            // mouse up = drone up, Mouse down = drone down
            glm::vec3 newPosition = glm::vec3(drone.position.x,
                drone.position.y - smoothedDeltaY * verticalSpeed,
                drone.position.z);

            // check collision before applying movement
            if (!DroneCollidesWithTerrain(newPosition) && !DroneCollidesWithObstacles(newPosition)) {
                drone.position.y = newPosition.y;
            }
            else if (DroneCollidesWithObstacles(newPosition)) {
                // bounce back if hitting obstacle
                drone.position.y += OBSTACLE_THROW_DISTANCE * verticalSpeed * (smoothedDeltaY > 0 ? -1 : 1);
            }
        }

        if (!timeExpired) {
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
        }

        glm::quat cameraRotationOY = glm::angleAxis(drone.angleOY, glm::vec3(0, 1, 0));
        glm::quat cameraRotationOX = glm::angleAxis(-0.2f, glm::vec3(1, 0, 0));
        glm::quat cameraRotation = cameraRotationOY * cameraRotationOX;

        glm::vec3 cameraOffset = cameraRotation * glm::vec3(0, CAMERA_TO_DRONE_DIST_OY, CAMERA_TO_DRONE_DIST_OX);
        SimpleScene::GetSceneCamera()->SetPosition(drone.position + cameraOffset);
        SimpleScene::GetSceneCamera()->SetRotation(cameraRotation);
    }
}

void DroneMinigame::OnKeyPress(int key, int mods)
{
}

void DroneMinigame::OnKeyRelease(int key, int mods)
{
}

void DroneMinigame::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
}

void DroneMinigame::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    if (!gameStarted) {
        if (button == GLFW_MOUSE_BUTTON_2) {
            gameStarted = true;
        }
    } else if (!couldGeneratePackageLocation || timeExpired) {
        if (button == GLFW_MOUSE_BUTTON_2) {
            playAgainClicked = true;
        }
    }    
}

void DroneMinigame::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
}

void DroneMinigame::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void DroneMinigame::OnWindowResize(int width, int height)
{
}
