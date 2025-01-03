#version 330

// get vertex attributes from each location
layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec3 vCoordinates;
layout(location = 3) in vec3 vColor;
layout(location = 4) in float vHeight;

// uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform vec3 terrainColor;

// output color for fragment shader
out vec3 vertexColor;

// 2D Random
float random(vec2 xz) {
	return fract(sin(dot(xz, vec2(12.9898, 78.233))) * 43758.5453123);
}

// 2D Noise based on Morgan McGuire @morgan3d
float noise(vec2 xz) {
	vec2 i = floor(xz);
	vec2 f = fract(xz);

	// Four corners in 2D of a tile
	float a = random(i);
	float b = random(i + vec2(1.0, 0.0));
	float c = random(i + vec2(0.0, 1.0));
	float d = random(i + vec2(1.0, 1.0));

	// Smooth Interpolation

	// Cubic Hermine Curve.  Same as SmoothStep()
	vec2 u = f * f * (vec2(3.0f) - f * 2.0f);
	// u = smoothstep(0.,1.,f);

	// Mix 4 coorners percentages
	return mix(a, b, u.x) +
		(c - a) * u.y * (1.0 - u.x) +
		(d - b) * u.x * u.y;
}

void main() {
	vec3 newPosition = vPosition;
	newPosition.y = vHeight;
	gl_Position = Projection * View * Model * vec4(newPosition, 1.0f);

	vec3 newColor = terrainColor;
	newColor.y = 0.5f + 0.5f*noise(vec2(vHeight, terrainColor.z));

	vertexColor = newColor;
}
