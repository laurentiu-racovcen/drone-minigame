#pragma once

#include "components/simple_scene.h"

class Random {
public:
	// 2D Random
	static float random(glm::vec2 xz) {
		return glm::fract(sin(dot(xz, glm::vec2(12.9898, 78.233))) * 43758.5453123);
	}

	// 2D Noise based on Morgan McGuire @morgan3d
	static float noise(glm::vec2 xz) {
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
};
