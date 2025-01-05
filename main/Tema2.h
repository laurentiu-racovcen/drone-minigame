#pragma once

#include "components/simple_scene.h"
#include "lab_m1/Tema2/meshes/transform3D.h"
#include "lab_m1/Tema2/drone/Drone.h"
#include "lab_m1/Tema2/terrain/Terrain.h"
#include "lab_m1/Tema2/obstacles/Tree.h"
#include "lab_m1/Tema2/obstacles/Building.h"
#include "lab_m1/Tema2/package/Package.h"
#include "lab_m1/Tema2/textbox/TextBox.h"

#define LEAVES_DISK_SCALE      3
#define TREE_TRUNK_HEIGHT      5
#define TREE_LEAVES_HEIGHT     3
#define LEAVES_DISTANCE        1

namespace m1
{
    class Tema2 : public gfxc::SimpleScene
    {
    public:
        Tema2();
        ~Tema2();

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
        void RenderPackageLocationDisk(glm::vec3 location);
        void RenderText(std::string text, float posX, float posY, float scale, glm::vec3 color, bool isAligned);

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

        void AddDroneMesh();
        void AddDronePropellerMesh();
        void AddTerrainMesh(Terrain *terrain);
        void AddTreeMesh();
        void AddBuildingMesh();
        void AddPackageMesh();
        void AddPackageLocationMesh();

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

        bool buildingIntersectsWithOtherBuilding(Building* currentBuilding, Building obstacleBuilding);
        Tree* generateRandomTree(Tree* currentTree);
        Building* generateRandomBuilding(Building* currentBuilding);
        void generateRandomTrees(unsigned int treesNum, unsigned int terrainWidth, unsigned int terrainLength);
        void generateRandomBuildings(unsigned int buildingsNum, unsigned int terrainWidth, unsigned int terrainLength);
        void generatePackageLocations();
        bool generateRandomPackage();
        bool packageIntersectsWithTree(glm::vec3 packagePosition, Tree obstacleTree);
        bool packageIntersectsWithBuilding(glm::vec3 packagePosition, Building obstacleBuilding);

    protected:
        //implemented::DroneCamera* camera;
        glm::mat4 projectionMatrix;
        Drone drone;
        Terrain terrain;
        vector<Tree> trees;
        vector<Building> buildings;
        Package package;
        unsigned int score;
    };
}   // namespace m1
