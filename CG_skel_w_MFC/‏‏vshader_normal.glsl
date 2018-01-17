#version 400

uniform mat4 modelViewMatrix;
uniform mat3 normalModelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelViewProjectionMatrix;

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 normal;
layout (location = 2) in bool isNormal;

void main() {
	if (isNormal) {
		vec3 transformedVertex = (modelViewMatrix * vec4(vertexPosition, 1)).xyz;
		vec3 transformedNormal = normalModelViewMatrix * normal;
		gl_Position = projectionMatrix * vec4(transformedVertex + transformedNormal, 1);
	} else {
		gl_Position = modelViewProjectionMatrix * vec4(vertexPosition, 1);
	}
}