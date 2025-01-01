#pragma once

#include "lab_m1/Tema2/main/Tema2.h"
#include "lab_m1/Tema2/meshes/transform3D.h"

using namespace std;
using namespace m1;

// Mesh colors
#define COLOR_GRAY  0.85, 0.85, 0.85
#define COLOR_BLACK    0,    0,    0

void Tema2::CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices)
{
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

    // Mesh information is saved into a Mesh object
    meshes[name] = new Mesh(name);
    meshes[name]->InitFromBuffer(VAO, static_cast<unsigned int>(indices.size()));
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
