#include "lab_m1/Tema2/terrain/Terrain.h"
#include "lab_m1/Tema2/random/Random.h"

#include <tuple>

using namespace std;

Terrain::Terrain()
{
}

Terrain::~Terrain()
{
}

// Method that generates random vertices heights in the range of (0, 1)
void Terrain::generateTerrainHeights() {
	for (unsigned int i = 0; i < (m+1) * (n+1); i++) {
		float currentNoiseValue = Random::noise(glm::vec2(this->vertices[i].position.x, this->vertices[i].position.z));
		this->verticesHeights.push_back(0.5f*currentNoiseValue);
		// also add vertex in map
		this->verticesHeightsMap.insert({ make_tuple(this->vertices[i].position.x, this->vertices[i].position.y, this->vertices[i].position.z), 0.5f * currentNoiseValue });
	}
}
