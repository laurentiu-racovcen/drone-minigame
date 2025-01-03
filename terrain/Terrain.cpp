#include "lab_m1/Tema2/terrain/Terrain.h"

Terrain::Terrain()
{
}

Terrain::~Terrain()
{
}

// 2D Random
float Terrain::random(glm::vec2 xz) {
	return glm::fract(sin(dot(xz, glm::vec2(12.9898, 78.233))) * 43758.5453123);
}

// 2D Noise based on Morgan McGuire @morgan3d
float Terrain::noise(glm::vec2 xz) {
	glm::vec2 i = floor(xz);
	glm::vec2 f = fract(xz);

	// Four corners in 2D of a tile
	float a = random(i);
	float b = random(i + glm::vec2(1.0, 0.0));
	float c = random(i + glm::vec2(0.0, 1.0));
	float d = random(i + glm::vec2(1.0, 1.0));

	// Smooth Interpolation

	// Cubic Hermine Curve.  Same as SmoothStep()
	glm::vec2 u = f * f * (glm::vec2(3.0f) - f * 2.0f);
	// u = smoothstep(0.,1.,f);

	// Mix 4 coorners percentages
	return glm::mix(a, b, u.x) +
		(c - a) * u.y * (1.0 - u.x) +
		(d - b) * u.x * u.y;
}

// Method that generates random vertices heights in the range of (0, 1)
void Terrain::generateTerrainHeights() {
	for (unsigned int i = 0; i < (m+1) * (n+1); i++) {
		float currentNoiseValue = this->noise(glm::vec2(this->vertices[i].position.x, this->vertices[i].position.z));
		this->verticesHeights.push_back(0.5f*currentNoiseValue);
	}
}
