#version 400

uniform mat4 viewProjectionMatrix;

layout (location = 0) in vec3 vertexPosition;

layout (location = 0) out vec3 outTextureCoordinates;

void main() {
	outTextureCoordinates = vertexPosition;
	gl_Position = viewProjectionMatrix * vec4(vertexPosition, 1);
}