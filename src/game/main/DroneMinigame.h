#pragma once

#include <chrono>
#include <iomanip>

#include "components/simple_scene.h"
#include "game/meshes/transform3D.h"
#include "game/drone/Drone.h"
#include "game/terrain/Terrain.h"
#include "game/obstacles/Tree.h"
#include "game/obstacles/Building.h"
#include "game/package/Package.h"
#include "game/basic_text/basic_text.h"
#include "game/random/Random.h"
#include "game/minimap/Minimap.h"

#define LEAVES_DISK_SCALE      3
#define TREE_TRUNK_HEIGHT      5
#define TREE_LEAVES_HEIGHT     3
#define LEAVES_DISTANCE        1

namespace m1
{
    class DroneMinigame : public gfxc::SimpleScene
    {
    public:
        DroneMinigame();
        ~DroneMinigame();

        void Init() override;

    private:
        void CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices);
        void CreateTerrainMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices);
        void RenderTerrainMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix);
        void RenderDrone(float deltaTimeSeconds);
        void RenderTerrain();
        void RenderTrees();
        void RenderBuildings();
        void RenderPackage();
        void RenderLocation();
        void RenderLocationOnMinimap();
        void RenderPackageLocationArrow();
        void RenderDroneLocationMinimapArrow();
        void RenderPackageSrcLocationDisk();
        void RenderPackageSrcLocationDiskMinimap();
        void RenderPackageDstLocationDisk();
        void RenderPackageDstLocationDiskMinimap();
        void RenderGameIntro();
        void DrawHUD();
        void showCouldNotGeneratePackageLocation();
        void showPlayAgainText();

        void FrameStart() override;
        void FrameEnd() override;
        void Update(float deltaTimeSeconds) override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        void AddAllMeshes();
        void AddDroneMesh();
        void AddDronePropellerMesh();
        void AddTerrainMesh(Terrain *terrain);
        void AddTreeMesh();
        void AddBuildingMesh();
        void AddPackageMesh();
        void AddPackageSrcLocationMesh();
        void AddPackageDstLocationMesh();
        void AddPackageLocationArrowMesh();
        void AddDroneLocationMinimapArrowMesh();

        bool treeIntersectsWithOtherTree(Tree *currentTree, Tree obstacleTree);
        bool treeIntersectsWithBuilding(Tree* currentTree, Building obstacleBuilding);
        bool DroneCollidesWithTerrain(glm::vec3 dronePosition);
        bool DroneCollidesWithMapLimits(glm::vec3 dronePosition);
        bool DroneCollidesWithATree(glm::vec3 dronePosition);
        bool DroneCollidesWithABuilding(glm::vec3 dronePosition);
        bool DroneCollidesWithObstacles(glm::vec3 dronePosition);
        bool DroneCollidesWithPackage();
        bool DroneCollidesWithDestPackage();
        bool disksIntersect(glm::vec3 dronePosition, glm::vec3 treePosition, float trunkRadius, float droneRadius);

        void StartNewRound();
        bool buildingIntersectsWithOtherBuilding(Building* currentBuilding, Building obstacleBuilding);
        Tree* generateRandomTree(Tree* currentTree);
        Building* generateRandomBuilding(Building* currentBuilding);
        void generateRandomTrees(unsigned int treesNum, unsigned int terrainWidth, unsigned int terrainLength);
        void generateRandomBuildings(unsigned int buildingsNum, unsigned int terrainWidth, unsigned int terrainLength);
        void generatePackageLocations();
        bool generateRandomPackage();
        bool packageIntersectsWithTree(glm::vec3 packagePosition, Tree obstacleTree);
        bool packageIntersectsWithBuilding(glm::vec3 packagePosition, Building obstacleBuilding);
        glm::vec3 getDroneInitialPosition();

        float getAngleBetweenPoints(glm::vec2 p1, glm::vec2 p2);

    protected:
        glm::mat4 projectionMatrix;
        Drone drone;
        Terrain terrain;
        vector<Tree> trees;
        vector<Building> buildings;
        Package package;
        unsigned int score;
        bool timeExpired;
        bool playAgainClicked;
        bool couldGeneratePackageLocation;
        bool gameStarted;
        bool gameInterrupted;
        bool cursorIsHidden;

        /* mini-map */
        Minimap minimap;

        /* for animated package locations disks */
        float packageLocationRadius;
        float packageLocationRadiusStep;

        /* for text */
        gfxc::TextRenderer* titleTextRenderer;
        gfxc::TextRenderer* subtextRenderer;
        gfxc::TextRenderer* roundTextRenderer;
        gfxc::TextRenderer* playAgainTextRenderer;
        const glm::vec3 kTextColor = NormalizedRGB(0, 0, 0);
        const glm::vec3 kBackgroundColor = NormalizedRGB(41, 45, 62);

        /* for cronometer */
        chrono::time_point<std::chrono::high_resolution_clock> startTime;
        chrono::time_point<std::chrono::high_resolution_clock> stopTime;
    };
}   // namespace m1
