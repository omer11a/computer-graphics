#version 400

uniform samplerCube cubeSampler;

layout (location = 0) in vec3 textureCoordinates;

layout (location = 0) out vec4 outColor;

void main() {
	outColor = texture(cubeSampler, textureCoordnates);
}