#pragma once

#include <iostream>

#include "lab_m1/Tema2/main/Tema2.h"
#include "lab_m1/Tema2/meshes/transform3D.h"

using namespace std;
using namespace m1;

// Mesh colors
#define COLOR_GRAY                   0.85, 0.85, 0.85
#define COLOR_BLACK                  0, 0, 0

#define DISK_TRIANGLES_NUM           50
#define TREE_TRUNK_COLOR             0.294, 0.224, 0.16
#define TREE_LEAVES_COLOR            0.455, 0.77, 0.363
#define TREE_TOP_LEAVES_COLOR        0.255, 0.27, 0.263
#define PACKAGE_LOCATION_DISK_COLOR  1, 1, 0

void Tema2::CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices)
{
    Mesh *newMesh = new Mesh(name);
    newMesh->vertices = vertices;
    newMesh->indices = indices;
    meshes[name] = newMesh;

    unsigned int VAO = 0;
    // Create and bind the VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO = 0;
    // Create and bind the VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Send vertices data into the VBO buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    unsigned int IBO = 0;
    // Create and bind the IBO
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    // Send indices data into the IBO buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

    // Set vertex position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), 0);

    // Set vertex normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec3)));

    // Set texture coordinate attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3)));

    // Set vertex color attribute
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3) + sizeof(glm::vec2)));

    // Unbind the VAO
    glBindVertexArray(0);

    // Check for OpenGL errors
    if (GetOpenGLError() == GL_INVALID_OPERATION)
    {
        printf("\tOpenGL error.\n");
    }

    meshes[name]->InitFromBuffer(VAO, static_cast<unsigned int>(indices.size()));
}

void Tema2::CreateTerrainMesh(const char* name, const std::vector<VertexFormat>& vertices,
                               const std::vector<unsigned int>& indices)
{
    Mesh* newMesh = new Mesh(name);
    newMesh->vertices = vertices;
    newMesh->indices = indices;
    meshes[name] = newMesh;

    unsigned int VAO = 0;
    // Create and bind the VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    unsigned int VBO = 0;
    // Create and bind the VBO
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Send vertices data into the VBO buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    unsigned int IBO = 0;
    // Create and bind the IBO
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    // Send indices data into the IBO buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

    // Set vertex position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), 0);

    // Set vertex normal attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(sizeof(glm::vec3)));

    // Set texture coordinate attribute
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3)));

    // Set vertex color attribute
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)(2 * sizeof(glm::vec3) + sizeof(glm::vec2)));

    // Create and bind a new VBO for vertex height attribute
    unsigned int heightsVBO = 0;
    glGenBuffers(1, &heightsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, heightsVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(terrain.verticesHeights[0]) * terrain.verticesHeights.size(), &terrain.verticesHeights[0], GL_STATIC_DRAW);

    // Set vertex height attribute
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(float), 0);

    // Unbind the VAO
    glBindVertexArray(0);

    // Check for OpenGL errors
    if (GetOpenGLError() == GL_INVALID_OPERATION)
    {
        printf("\tOpenGL error.\n");
    }

    meshes[name]->InitFromBuffer(VAO, static_cast<unsigned int>(indices.size()));
}

void Tema2::RenderTerrainMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    // Render an object using the specified shader and the specified position
    glUseProgram(shader->program);

    // Get shader location for uniform mat4 "Model"
    int modelLocation = glGetUniformLocation(shader->program, "Model");

    // Set shader uniform "Model" to modelMatrix
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Get shader location for uniform mat4 "View"
    int viewLocation = glGetUniformLocation(shader->program, "View");

    // Set shader uniform "View" to viewMatrix
    glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    // Get shader location for uniform mat4 "Projection"
    int projectionLocation = glGetUniformLocation(shader->program, "Projection");

    // Set shader uniform "Projection" to projectionMatrix
    glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    // Get the mesh color and send it to the shader
    glUniform3fv(glGetUniformLocation(shader->program, "terrainColor"), 1, glm::value_ptr(terrain.color));

    // Draw the object
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}

void Tema2::AddDroneMesh()
{
    vector<VertexFormat> vertices
    {
        /* 1st part */
    
        // for the parallelepiped
        VertexFormat(glm::vec3(1, -0.1, -0.1), glm::vec3(COLOR_GRAY)),    // 0
        VertexFormat(glm::vec3(1, 0.1, -0.1), glm::vec3(COLOR_GRAY)),     // 1
        VertexFormat(glm::vec3(1, -0.1, 0.1), glm::vec3(COLOR_GRAY)),     // 2
        VertexFormat(glm::vec3(1, 0.1, 0.1), glm::vec3(COLOR_GRAY)),      // 3

        // for the cube
        VertexFormat(glm::vec3(1, 0.3, 0.1), glm::vec3(COLOR_GRAY)),      // top - 4
        VertexFormat(glm::vec3(1, 0.3, -0.1), glm::vec3(COLOR_GRAY)),     // top - 5
        VertexFormat(glm::vec3(0.8, 0.1, 0.1), glm::vec3(COLOR_GRAY)),    // bottom - 6
        VertexFormat(glm::vec3(0.8, 0.1, -0.1), glm::vec3(COLOR_GRAY)),   // bottom - 7
        VertexFormat(glm::vec3(0.8, 0.3, 0.1), glm::vec3(COLOR_GRAY)),    // first corner - 8
        VertexFormat(glm::vec3(0.8, 0.3, -0.1), glm::vec3(COLOR_GRAY)),   // second corner - 9

        /* 2nd part */

        // for the parallelepiped
        VertexFormat(glm::vec3(-1, -0.1, 0.1), glm::vec3(COLOR_GRAY)),    // 10
        VertexFormat(glm::vec3(-1, -0.1, -0.1), glm::vec3(COLOR_GRAY)),   // 11
        VertexFormat(glm::vec3(-1, 0.1, -0.1), glm::vec3(COLOR_GRAY)),    // 12
        VertexFormat(glm::vec3(-1, 0.1, 0.1), glm::vec3(COLOR_GRAY)),     // 13

        // for the cube
        VertexFormat(glm::vec3(-1, 0.3, -0.1), glm::vec3(COLOR_GRAY)),    // top - 14
        VertexFormat(glm::vec3(-1, 0.3, 0.1), glm::vec3(COLOR_GRAY)),     // top - 15
        VertexFormat(glm::vec3(-0.8, 0.1, 0.1), glm::vec3(COLOR_GRAY)),   // bottom - 16
        VertexFormat(glm::vec3(-0.8, 0.1, -0.1), glm::vec3(COLOR_GRAY)),  // bottom - 17
        VertexFormat(glm::vec3(-0.8, 0.3, 0.1), glm::vec3(COLOR_GRAY)),   // 1st corner - 18
        VertexFormat(glm::vec3(-0.8, 0.3, -0.1), glm::vec3(COLOR_GRAY)),  // 2nd corner - 19

        /* 3rd part */

        // for the parallelepiped
        VertexFormat(glm::vec3(-0.1, 0.1, -1), glm::vec3(COLOR_GRAY)),    // top right - 20
        VertexFormat(glm::vec3(0.1, 0.1, -1), glm::vec3(COLOR_GRAY)),     // top left - 21
        VertexFormat(glm::vec3(-0.1, -0.1, -1), glm::vec3(COLOR_GRAY)),   // bottom-right - 22
        VertexFormat(glm::vec3(0.1, -0.1, -1), glm::vec3(COLOR_GRAY)),    // bottom-left - 23

        // for the cube
        VertexFormat(glm::vec3(-0.1, 0.3, -1), glm::vec3(COLOR_GRAY)),     // top - 24
        VertexFormat(glm::vec3(0.1, 0.3, -1), glm::vec3(COLOR_GRAY)),      // top - 25
        VertexFormat(glm::vec3(-0.1, 0.1, -0.8), glm::vec3(COLOR_GRAY)),   // bottom - 26
        VertexFormat(glm::vec3(0.1, 0.1, -0.8), glm::vec3(COLOR_GRAY)),    // bottom - 27
        VertexFormat(glm::vec3(-0.1, 0.3, -0.8), glm::vec3(COLOR_GRAY)),   // 1st corner - 28
        VertexFormat(glm::vec3(0.1, 0.3, -0.8), glm::vec3(COLOR_GRAY)),    // 2nd corner - 29

        /* 4th part */

        // for the parallelepiped
        VertexFormat(glm::vec3(0.1, 0.1, 1), glm::vec3(COLOR_GRAY)),      // top right - 30
        VertexFormat(glm::vec3(-0.1, 0.1, 1), glm::vec3(COLOR_GRAY)),     // top left - 31
        VertexFormat(glm::vec3(0.1, -0.1, 1), glm::vec3(COLOR_GRAY)),     // bottom-right - 32
        VertexFormat(glm::vec3(-0.1, -0.1, 1), glm::vec3(COLOR_GRAY)),    // bottom-left - 33

        // for the cube
        VertexFormat(glm::vec3(-0.1, 0.3, 1), glm::vec3(COLOR_GRAY)),     // top - 34
        VertexFormat(glm::vec3(0.1, 0.3, 1), glm::vec3(COLOR_GRAY)),      // top - 35
        VertexFormat(glm::vec3(-0.1, 0.1, 0.8), glm::vec3(COLOR_GRAY)),   // bottom - 36
        VertexFormat(glm::vec3(0.1, 0.1, 0.8), glm::vec3(COLOR_GRAY)),    // bottom - 37
        VertexFormat(glm::vec3(-0.1, 0.3, 0.8), glm::vec3(COLOR_GRAY)),   // 1st corner - 38
        VertexFormat(glm::vec3(0.1, 0.3, 0.8), glm::vec3(COLOR_GRAY)),    // 2nd corner - 39

    };

    vector<unsigned int> indices
    {
        /* for the 1st parallelepiped */

        // 1st exterior side
        2, 0, 1,
        2, 1, 3,

        // right side
        3, 1, 12,
        3, 12, 13,

        // left side
        10, 11, 0,
        10, 0, 2,

        // top side
        3, 13, 2,
        3, 10, 2,

        // bottom side
        0, 11, 12,
        0, 12, 1,

        // 2nd exterior side
        13, 12, 11,
        13, 11, 10,

        /* for the 2nd parallelepiped */
        
        // 1st exterior side
        32, 33, 31,
        32, 31, 30,

        // right side
        30, 32, 23,
        30, 23, 21,

        // left side
        20, 22, 33,
        20, 31, 33,

        // top side
        31, 30, 21,
        31, 21, 20,

        // bottom side
        32, 33, 22,
        32, 22, 23,

        // 2nd exterior side
        22, 23, 21,
        22, 21, 20,

        /* ---------- cube 1 ---------- */

        // side 1
        4, 3, 1,
        4, 1, 5,

        // side 2
        9, 7, 6,
        9, 6, 8,

        // side 3
        9, 7, 1,
        9, 1, 5,

        // side 4
        8, 6, 3,
        8, 3, 4,

        // top side
        8, 4, 5,
        8, 5, 9,

        /* ---------- cube 2 ---------- */

        // side 1
        14, 12, 13,
        14, 13, 15,

        // side 2
        15, 13, 16,
        15, 16, 18,

        // side 3
        18, 16, 17,
        18, 17, 19,

        // side 4
        19, 17, 12,
        19, 12, 14,

        // top side
        14, 15, 18,
        14, 18, 19,

        /* ---------- cube 3 ---------- */

        // side 1
        25, 21, 20,
        25, 20, 24,

        // side 2
        24, 20, 26,
        24, 26, 28,

        // side 3
        28, 26, 27,
        28, 27, 29,

        // side 4
        29, 27, 21,
        29, 21, 25,

        // top
        25, 24, 28,
        25, 28, 29,

        /* ---------- cube 4 ---------- */

        // side 1
        34, 31, 30,
        34, 30, 35,

        // side 2
        35, 30, 37,
        35, 37, 39,

        // side 3
        39, 37, 36,
        39, 36, 38,

        // side 4
        38, 36, 31,
        38, 31, 34,

        // top
        38, 34, 35,
        38, 35, 39,
    };

    // Create the mesh from the data
    CreateMesh("drone", vertices, indices);
}

void Tema2::AddDronePropellerMesh()
{
    vector<VertexFormat> vertices
    {
        VertexFormat(glm::vec3(-0.2, -0.035, 0), glm::vec3(COLOR_BLACK)),    // 0
        VertexFormat(glm::vec3(-0.2, 0.035, 0), glm::vec3(COLOR_BLACK)),     // 1
        VertexFormat(glm::vec3(0.2, 0.035, 0), glm::vec3(COLOR_BLACK)),      // 2
        VertexFormat(glm::vec3(0.2, -0.035, 0), glm::vec3(COLOR_BLACK)),     // 3
    };

    vector<unsigned int> indices
    {
        0, 1, 2,
        0, 2, 3,
    };

    // Create the mesh from the data
    CreateMesh("drone-propeller", vertices, indices);
}

void Tema2::AddTerrainMesh(Terrain *terrain)
{
    vector<VertexFormat> vertices;

    for (unsigned int i = 0; i <= terrain->m; i++) {
        for (unsigned int j = 0; j <= terrain->n; j++) {
            vertices.push_back(VertexFormat(glm::vec3(j, 0, i), terrain->color));
            // also add current vertex in the terrain instance vertices vector
            terrain->vertices.push_back(VertexFormat(glm::vec3(j, 0, i), terrain->color));
        }
    }

    vector<unsigned int> indices;

    for (unsigned int i = 0; i < terrain->m; i++) {
        for (unsigned int j = 0; j < terrain->n; j++) {
            // 1st triangle of current rectangle (bottom left)
            indices.push_back((terrain->n + 1) * i + j);
            indices.push_back((terrain->n + 1) * (i + 1) + j);
            indices.push_back((terrain->n + 1) * (i + 1) + j + 1);

            // 2nd triangle of current rectangle (top right)
            indices.push_back((terrain->n + 1) * i + j);
            indices.push_back((terrain->n + 1) * (i + 1) + j + 1);
            indices.push_back((terrain->n + 1) * i + j + 1);
        }
    }

    // fill the random heights vector
    terrain->generateTerrainHeights();

    // Create the mesh from the data
    CreateTerrainMesh("terrain", vertices, indices);
}

void Tema2::AddTreeMesh()
{
    /* ----------- compute tree trunk mesh ----------- */

    unsigned int k = DISK_TRIANGLES_NUM;
    vector<VertexFormat> vertices;
    vector<unsigned int> indices;
    // initialize the first vertex (x,y) = (1,0) in the vertices vector

    // add origin of (x,y) = (0, 0)
    vertices.push_back(VertexFormat(glm::vec3(0, 0, 0), glm::vec3(TREE_TRUNK_COLOR)));

    // insert all the vertices of the top part of the trunk
    for (unsigned int i = 1; i <= k; i++) {
        vertices.push_back(VertexFormat(glm::vec3(cos(((float)i / k) * 2 * 3.14f)/2, 0, sin(((float)i / k) * 2 * 3.14f)/2),
            glm::vec3(TREE_TRUNK_COLOR)));

    }

    // insert all the indices of the disk
    for (unsigned int i = 2; i <= k; i++) {
        indices.push_back(i);
        indices.push_back(0);
        indices.push_back(i - 1);
    }

    // add last triangle indices of the disk
    indices.push_back(1);
    indices.push_back(0);
    indices.push_back(k);

    unsigned int bottomDiskVerticesNum = vertices.size();

    // insert all the vertices of the top part of the trunk
    for (unsigned int i = 1; i < bottomDiskVerticesNum-1; i++) {
        /* compute current rectangle vertices coordinates */

        // bottom-left triangle

        // top-left vertex
        vertices.push_back(VertexFormat(
            glm::vec3(vertices[i].position.x, vertices[i].position.y + TREE_TRUNK_HEIGHT, vertices[i].position.z),
                glm::vec3(TREE_TRUNK_COLOR)
            )
        );

        // bottom-left vertex
        vertices.push_back(VertexFormat(
                glm::vec3(vertices[i].position.x, vertices[i].position.y, vertices[i].position.z),
                glm::vec3(TREE_TRUNK_COLOR)
            )
        );

        // bottom-right vertex
        vertices.push_back(VertexFormat(
                glm::vec3(vertices[i+1].position.x, vertices[i+1].position.y, vertices[i+1].position.z),
                glm::vec3(TREE_TRUNK_COLOR)
            )
        );

        // top-right triangle

        // top-left vertex
        vertices.push_back(VertexFormat(
                glm::vec3(vertices[i].position.x, vertices[i].position.y + TREE_TRUNK_HEIGHT, vertices[i].position.z),
                glm::vec3(TREE_TRUNK_COLOR)
            )
        );

        // bottom-right vertex
        vertices.push_back(VertexFormat(
                glm::vec3(vertices[i + 1].position.x, vertices[i + 1].position.y, vertices[i + 1].position.z),
                glm::vec3(TREE_TRUNK_COLOR)
            )
        );

        // top-right vertex
        vertices.push_back(VertexFormat(
                glm::vec3(vertices[i + 1].position.x, vertices[i + 1].position.y + TREE_TRUNK_HEIGHT, vertices[i + 1].position.z),
                glm::vec3(TREE_TRUNK_COLOR)
            )
        );
    }

    // insert all the indices of the trunk
    for (unsigned int i = bottomDiskVerticesNum; i < vertices.size(); i++) {
        indices.push_back(i);
    }

    // insert last trunk rectangle indices
    indices.push_back(bottomDiskVerticesNum);
    indices.push_back(bottomDiskVerticesNum + 1);
    indices.push_back(vertices.size() - 2);

    indices.push_back(bottomDiskVerticesNum);
    indices.push_back(vertices.size() - 2);
    indices.push_back(vertices.size() - 1);

    /* ------------------- TREE LEAVES ---------------------- */

    /* FIRST CONE */

    unsigned int oldVerticesNum = vertices.size();
    // add origin of disk (x,y) = (0, 5, 0)
    vertices.push_back(VertexFormat(glm::vec3(0, TREE_TRUNK_HEIGHT, 0), glm::vec3(TREE_TOP_LEAVES_COLOR)));

    // add the top of cone (x,y) = (0, 8, 0)
    vertices.push_back(VertexFormat(glm::vec3(0, TREE_TRUNK_HEIGHT + TREE_LEAVES_HEIGHT, 0), glm::vec3(TREE_TOP_LEAVES_COLOR)));

    // insert all the vertices of the top part of the trunk
    for (unsigned int i = 1; i <= k; i++) {
        vertices.push_back(VertexFormat(glm::vec3(cos(((float)i / k) * 2 * 3.14f) * LEAVES_DISK_SCALE, TREE_TRUNK_HEIGHT, sin(((float)i / k) * 2 * 3.14f) * LEAVES_DISK_SCALE),
            glm::vec3(TREE_LEAVES_COLOR)));
    }

    // insert all the indices of the disk and cone
    for (unsigned int i = oldVerticesNum + 2; i <= oldVerticesNum + k + 1; i++) {
        indices.push_back(i);
        indices.push_back(oldVerticesNum);
        indices.push_back(i - 1);

        indices.push_back(i);
        indices.push_back(oldVerticesNum + 1);
        indices.push_back(i - 1);
    }

    // add last triangle indices of the cone disk
    indices.push_back(oldVerticesNum + 2);
    indices.push_back(oldVerticesNum);
    indices.push_back(oldVerticesNum + k + 1);

    // add last triangle indices of the cone
    indices.push_back(oldVerticesNum + 2);
    indices.push_back(oldVerticesNum + 1);
    indices.push_back(oldVerticesNum + k + 1);

    /* SECOND CONE */

    oldVerticesNum = vertices.size();
    // add origin of disk (x,y)
    vertices.push_back(VertexFormat(glm::vec3(0, TREE_TRUNK_HEIGHT + LEAVES_DISTANCE, 0), glm::vec3(TREE_TOP_LEAVES_COLOR)));

    // add the top of cone (x,y)
    vertices.push_back(VertexFormat(glm::vec3(0, TREE_TRUNK_HEIGHT + LEAVES_DISTANCE + TREE_LEAVES_HEIGHT, 0), glm::vec3(TREE_TOP_LEAVES_COLOR)));

    // insert all the vertices of the current leaves disk
    for (unsigned int i = 1; i <= k; i++) {
        vertices.push_back(VertexFormat(glm::vec3(cos(((float)i / k) * 2 * 3.14f) * LEAVES_DISK_SCALE, TREE_TRUNK_HEIGHT + LEAVES_DISTANCE, sin(((float)i / k) * 2 * 3.14f) * LEAVES_DISK_SCALE),
            glm::vec3(TREE_LEAVES_COLOR)));
    }

    // insert all the indices of the disk and cone
    for (unsigned int i = oldVerticesNum + 2; i <= oldVerticesNum + k + 1; i++) {
        indices.push_back(i);
        indices.push_back(oldVerticesNum);
        indices.push_back(i - 1);

        indices.push_back(i);
        indices.push_back(oldVerticesNum + 1);
        indices.push_back(i - 1);
    }

    // add last triangle indices of the cone disk
    indices.push_back(oldVerticesNum + 2);
    indices.push_back(oldVerticesNum);
    indices.push_back(oldVerticesNum + k + 1);

    // add last triangle indices of the cone
    indices.push_back(oldVerticesNum + 2);
    indices.push_back(oldVerticesNum + 1);
    indices.push_back(oldVerticesNum + k + 1);

    /* THIRD CONE */

    oldVerticesNum = vertices.size();
    // add origin of disk (x,y)
    vertices.push_back(VertexFormat(glm::vec3(0, TREE_TRUNK_HEIGHT + 2*LEAVES_DISTANCE, 0), glm::vec3(TREE_TOP_LEAVES_COLOR)));

    // add the top of cone (x,y)
    vertices.push_back(VertexFormat(glm::vec3(0, TREE_TRUNK_HEIGHT + 2*LEAVES_DISTANCE + TREE_LEAVES_HEIGHT, 0), glm::vec3(TREE_TOP_LEAVES_COLOR)));

    // insert all the vertices of the current leaves disk
    for (unsigned int i = 1; i <= k; i++) {
        vertices.push_back(VertexFormat(glm::vec3(cos(((float)i / k) * 2 * 3.14f) * LEAVES_DISK_SCALE, TREE_TRUNK_HEIGHT + 2*LEAVES_DISTANCE, sin(((float)i / k) * 2 * 3.14f) * LEAVES_DISK_SCALE),
            glm::vec3(TREE_LEAVES_COLOR)));
    }

    // insert all the indices of the disk and cone
    for (unsigned int i = oldVerticesNum + 2; i <= oldVerticesNum + k + 1; i++) {
        indices.push_back(i);
        indices.push_back(oldVerticesNum);
        indices.push_back(i - 1);

        indices.push_back(i);
        indices.push_back(oldVerticesNum + 1);
        indices.push_back(i - 1);
    }

    // add last triangle indices of the cone disk
    indices.push_back(oldVerticesNum + 2);
    indices.push_back(oldVerticesNum);
    indices.push_back(oldVerticesNum + k + 1);

    // add last triangle indices of the cone
    indices.push_back(oldVerticesNum + 2);
    indices.push_back(oldVerticesNum + 1);
    indices.push_back(oldVerticesNum + k + 1);

    // Create the mesh from the data
    CreateMesh("tree", vertices, indices);
}

void Tema2::AddBuildingMesh()
{
    vector<VertexFormat> vertices
    {
        // Front
        VertexFormat(glm::vec3(-0.5, 0, 0.5), glm::vec3(0.5f, 0.5f, 0.5f)),   // Bottom-left     0
        VertexFormat(glm::vec3(0.5, 0,  0.5), glm::vec3(0.8f, 0.8f, 0.8f)),   // Bottom-right   1
        VertexFormat(glm::vec3(0.5,  1,  0.5), glm::vec3(0.5f, 0.5f, 0.5f)),   // Top-right      2
        VertexFormat(glm::vec3(-0.5,  1,  0.5), glm::vec3(0.5f, 0.5f, 0.5f)),  // Top-left      3

        // Back
        VertexFormat(glm::vec3(-0.5, 0, -0.5), glm::vec3(0.5f, 0.5f, 0.5f)),  // Bottom-left  4
        VertexFormat(glm::vec3(0.5, 0, -0.5), glm::vec3(0.5f, 0.5f, 0.5f)),   // Bottom-right  5
        VertexFormat(glm::vec3(0.5,  1, -0.5), glm::vec3(0.5f, 0.5f, 0.5f)),   // Top-right     6
        VertexFormat(glm::vec3(-0.5,  1, -0.5), glm::vec3(0.8f, 0.8f, 0.8f)),  // Top-left     7
    };

    vector<unsigned int> indices =
    {
        0, 1, 2,  // Front face (clockwise)
        2, 3, 0,

        6, 5, 4,  // Back face (counterclockwise)
        4, 7, 6,

        0, 3, 4,  // Left face (clockwise)
        4, 3, 7,

        1, 6, 2,  // Right face (counterclockwise)
        5, 6, 1,

        5, 1, 0,  // Bottom face (clockwise)
        5, 0, 4,

        7, 3, 2,  // Top face (counterclockwise)
        6, 7, 2
    };

    // Create the mesh from the data
    CreateMesh("building", vertices, indices);
}

void Tema2::AddPackageMesh()
{
    vector<VertexFormat> vertices
    {
        // Front
        VertexFormat(glm::vec3(-0.5, 0, 0.5), glm::vec3(1, 0.5f, 0.5f)),   // Bottom-left     0
        VertexFormat(glm::vec3(0.5, 0,  0.5), glm::vec3(1, 0.8f, 0.8f)),   // Bottom-right   1
        VertexFormat(glm::vec3(0.5,  1,  0.5), glm::vec3(1, 0.5f, 0.5f)),   // Top-right      2
        VertexFormat(glm::vec3(-0.5,  1,  0.5), glm::vec3(1, 0.5f, 0.5f)),  // Top-left      3

        // Back
        VertexFormat(glm::vec3(-0.5, 0, -0.5), glm::vec3(1, 0.5f, 0.5f)),  // Bottom-left  4
        VertexFormat(glm::vec3(0.5, 0, -0.5), glm::vec3(1, 0.5f, 0.5f)),   // Bottom-right  5
        VertexFormat(glm::vec3(0.5,  1, -0.5), glm::vec3(1, 0.5f, 0.5f)),   // Top-right     6
        VertexFormat(glm::vec3(-0.5,  1, -0.5), glm::vec3(1, 0.8f, 0.8f)),  // Top-left     7
    };

    vector<unsigned int> indices =
    {
        0, 1, 2,  // Front face (clockwise)
        2, 3, 0,

        6, 5, 4,  // Back face (counterclockwise)
        4, 7, 6,

        0, 3, 4,  // Left face (clockwise)
        4, 3, 7,

        1, 6, 2,  // Right face (counterclockwise)
        5, 6, 1,

        5, 1, 0,  // Bottom face (clockwise)
        5, 0, 4,

        7, 3, 2,  // Top face (counterclockwise)
        6, 7, 2
    };

    // Create the mesh from the data
    CreateMesh("package", vertices, indices);
}

void Tema2::AddPackageLocationMesh()
{
    unsigned int k = 50;
    vector<VertexFormat> vertices;
    vector<unsigned int> indices;

    // add origin of (x,z) = (0, 0)
    vertices.push_back(VertexFormat(glm::vec3(0, 0, 0), glm::vec3(PACKAGE_LOCATION_DISK_COLOR)));

    // insert all the vertices of the disk
    for (unsigned int i = 1; i <= k; i++) {
        vertices.push_back(VertexFormat(glm::vec3(cos(((float)i / k) * 2 * 3.14f), 0, sin(((float)i / k) * 2 * 3.14f)),
            glm::vec3(PACKAGE_LOCATION_DISK_COLOR)));
    }

    // insert all the indices of the disk
    for (unsigned int i = 2; i <= k; i++) {
        indices.push_back(i);
        indices.push_back(0);
        indices.push_back(i - 1);
    }

    // add last triangle indices of the disk
    indices.push_back(1);
    indices.push_back(0);
    indices.push_back(k);

    // Actually create the mesh from the data
    CreateMesh("package-location-disk", vertices, indices);
}
