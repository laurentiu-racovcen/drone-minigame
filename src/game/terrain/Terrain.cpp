#include "game/terrain/Terrain.h"
#include "game/random/Random.h"

#include <tuple>

using namespace std;

Terrain::Terrain()
{
}

Terrain::~Terrain()
{
}

// Method that generates random vertices heights in the range of (0, MAX_TERRAIN_HEIGHT)
void Terrain::generateTerrainHeights() {
	for (unsigned int i = 0; i < (m+1) * (n+1); i++) {
		float currentNoiseValue = Random::noise(glm::vec2(this->vertices[i].position.x, this->vertices[i].position.z));
		this->verticesHeights.push_back(MAX_TERRAIN_HEIGHT * currentNoiseValue);
		// also add vertex in map
		this->verticesHeightsMap.insert(
			{
				make_tuple((int)this->vertices[i].position.x, (int)this->vertices[i].position.y, (int)this->vertices[i].position.z),
				MAX_TERRAIN_HEIGHT * currentNoiseValue 
			}
		);
	}
}
