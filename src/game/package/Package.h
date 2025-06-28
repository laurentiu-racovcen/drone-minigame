#pragma once

#include "components/simple_scene.h"

class Package {
public:
	glm::vec3 sourceLocation;
	glm::vec3 destinationLocation;
	float scale;
	bool isInTransit;
};
