#pragma once

#include <vector>
#include "components/simple_scene.h"

using namespace std;

class Terrain {
public:
	Terrain();
	~Terrain();
	Terrain(unsigned int m, unsigned int n, glm::vec3 color) {
		this->m = m;
		this->n = n;
		this->color = color;
	}

public:
	float noise(glm::vec2 xz);
	void generateTerrainHeights();
	float random(glm::vec2 xz);

public:
	glm::vec3 color;
	unsigned int m;
	unsigned int n;
	vector <VertexFormat> vertices;
	vector <float> verticesHeights;
};
