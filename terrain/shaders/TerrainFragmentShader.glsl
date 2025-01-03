#version 330

// get color from vertex shader
in vec3 vertexColor;

// output color
layout(location = 0) out vec4 fragColor;

void main() {
	fragColor = vec4(vertexColor, 1);
}
