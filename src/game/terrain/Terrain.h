#pragma once

#include <vector>
#include <map>
#include "components/simple_scene.h"

#define MAX_TERRAIN_HEIGHT  0.5f

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
	void generateTerrainHeights();

public:
	glm::vec3 color;
	unsigned int m;
	unsigned int n;
	vector <VertexFormat> vertices;
	vector <float> verticesHeights;
	map <tuple<int, int, int>, float> verticesHeightsMap;
};
