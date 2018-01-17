#version 400

uniform vec3 color;

layout (location = 0) out outColor;

void main() {
	outColor = color;
}