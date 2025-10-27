#version 460 core
layout (location = 0) in vec3 position;

uniform mat4 projectionViewModelMatrix;
uniform mat4 projectionMatrix;
uniform mat4 viewModelMatrix;

uniform float normalOffset = 0.0001;

void main() {
	vec4 pos = viewModelMatrix * vec4(position, 1.0);
	pos.z += normalOffset;
	gl_Position = projectionMatrix * pos;
}