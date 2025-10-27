#version 460 core

uniform vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

out vec4 fragColor;  

void main() {
	fragColor = color;
}