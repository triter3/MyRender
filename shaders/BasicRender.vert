#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 projectionViewModelMatrix;
uniform vec4 outColor = vec4(0.8, 0.0, 0.0, 1.0);

out vec4 fcolor;

void main() {
	gl_Position = projectionViewModelMatrix * vec4(position, 1.0f);
	fcolor = outColor;
}
