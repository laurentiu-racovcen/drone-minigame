#pragma once

#include "components/simple_scene.h"
#include "lab_m1/Tema2/meshes/transform3D.h"
#include "lab_m1/Tema2/drone/Drone.h"
#include "lab_m1/Tema2/terrain/Terrain.h"

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
        void AddTreeMesh(float scale);

    protected:
        //implemented::DroneCamera* camera;
        glm::mat4 projectionMatrix;
        Drone drone;
        Terrain terrain;
    };
}   // namespace m1
